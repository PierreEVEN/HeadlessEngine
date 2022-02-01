#include "ecs/actor_meta_data.h"

#include "ecs/ecs.h"

namespace ecs
{

ActorVariant::ActorVariant(ECS* context, const std::vector<ComponentTypeID>& in_specification) : specification(in_specification), owning_ecs(context)
{
    // Initialize component's memory
    components.resize(in_specification.size());
    for (size_t i = 0; i < in_specification.size(); ++i)
    {
        IComponentHelper* component = ECS::component_registry[in_specification[i]];
        if (!component)
            LOG_FATAL("component with id %lu is not registered", in_specification[i])

        components[i] = ComponentData{
            .type_size             = component->type_size(),
            .component_data_buffer = {},
            .type_id               = in_specification[i],
            .component_type        = component,
        };
    }
}

void ActorVariant::emplace_actor_back(ActorMetaData* actor)
{
    // Emplace new actor
    actor->variant    = this;
    actor->data_index = static_cast<uint32_t>(linked_actors.size());
    linked_actors.emplace_back(actor->actor_id);
    // Resize get memories
    update_components_buffer_size();
}

void ActorVariant::remove_actor(ActorMetaData* actor, bool only_move)
{
    for (auto& component : components)
    {
        const auto removed_component_memory = &component.component_data_buffer[(linked_actors.size() - 1) * component.type_size];
        if (!only_move)
        {
            // Call destructor
            component.component_type->component_destroy(removed_component_memory);
        }

        // Move last component's data into the removed one to erase it
        const auto last_component_memory    = &component.component_data_buffer[actor->data_index * component.type_size];
        component.component_type->component_move(last_component_memory, removed_component_memory);
    }

    // Update references
    const ActorID& last_actor_id                         = *(linked_actors.end() - 1);
    uint32_t       removed_index                         = actor->data_index;
    owning_ecs->actor_registry[last_actor_id].data_index = removed_index;
    linked_actors[removed_index]                         = last_actor_id;
    linked_actors.pop_back();

    actor->variant    = nullptr;
    actor->data_index = UINT32_MAX;

    update_components_buffer_size();
}

void ActorVariant::move_actor_to_variant(ActorMetaData* actor, ActorVariant* from, ActorVariant* to)
{
    const size_t previous_data_index = actor->data_index;
    const size_t new_data_index      = to->linked_actors.size();

    ActorMetaData old_actor_infos = *actor;

    // Register into new variant
    to->emplace_actor_back(actor);

    // Move existing component data
    for (auto& new_component : to->components)
        for (auto& old_component : from->components)
            if (new_component.type_id == old_component.type_id)
            {
                new_component.component_type->component_move(&old_component.component_data_buffer[previous_data_index * old_component.type_size],
                                                             &new_component.component_data_buffer[new_data_index * new_component.type_size]);
                break;
            }

    // Remove from the previous variant
    from->remove_actor(&old_actor_infos, true);

    // Update references
    actor->variant    = to;
    actor->data_index = static_cast<uint32_t>(new_data_index);
}

void ActorVariant::duplicate_actor(const ActorMetaData* existing_actor, ActorMetaData* new_actor)
{
    // Emplace new actor
    new_actor->variant    = this;
    new_actor->data_index = static_cast<uint32_t>(linked_actors.size());
    linked_actors.emplace_back(new_actor->actor_id);

    // Resize get memories
    update_components_buffer_size();

    // Move last component's data into the removed one to erase it
    for (auto& component : components)
    {
        const auto* existing_component_memory = &component.component_data_buffer[existing_actor->data_index * component.type_size];
        auto*       new_component_memory      = &component.component_data_buffer[(linked_actors.size() - 1) * component.type_size];
        memcpy(new_component_memory, existing_component_memory, sizeof(component.type_size));
    }
}

void ActorVariant::update_components_buffer_size()
{
    for (auto& component : components)
    {
        component.component_data_buffer.resize(linked_actors.size() * component.type_size);
    }
}
} // namespace ecs