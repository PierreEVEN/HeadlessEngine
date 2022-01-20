#include "ecs/actor_meta_data.h"

#include "ecs/ecs.h"

namespace ecs
{
ActorVariant::ActorVariant(const std::vector<ComponentTypeID>& in_specification)
{
    components.resize(in_specification.size());
    for (size_t i = 0; i < in_specification.size(); ++i)
    {
        IComponentHelper* component = singleton().get_component_type(in_specification[i]);
        if (!component)
            LOG_FATAL("component with id %lu is not registered", in_specification[i])

        components[i] = ComponentData{
            .type_size      = component->type_size(),
            .component_data = {},
            .type_id        = in_specification[i],
            .component_type = component,
        };
    }
}

void ActorVariant::add_actor(ActorMetaData* actor)
{
    actor->variant    = this;
    actor->data_index = static_cast<uint32_t>(linked_actors.size());

    linked_actors.emplace_back(actor->actor_id);

    for (auto& component : components)
        component.component_data.resize(linked_actors.size() * component.type_size);
}

void ActorVariant::remove_actor(ActorMetaData* actor)
{
    uint32_t removed_index = actor->data_index;

    // Move last component's data into the removed one to erase it
    for (auto& component : components)
        component.component_type->component_move(&component.component_data[actor->data_index * component.type_size], &component.component_data[(linked_actors.size() - 1) * component.type_size]);

    const auto last_ptr = linked_actors.end() - 1;

    singleton().actor_registry[*last_ptr].data_index = removed_index;
    linked_actors[removed_index]                    = *last_ptr;
    linked_actors.pop_back();

    for (auto& component : components)
        component.component_data.resize(linked_actors.size() * component.type_size);

    actor->variant    = nullptr;
    actor->data_index = 0;
}

void ActorVariant::copy_to_this_variant(ActorMetaData* actor, ActorVariant* previous_variant)
{
    const size_t previous_data_index = actor->data_index;
    add_actor(actor);
    const size_t new_data_index = actor->data_index;

    for (auto& new_component : components)
        for (auto& old_component : previous_variant->components)
            if (new_component.type_id == old_component.type_id)
            {
                new_component.component_type->component_move(&old_component.component_data[previous_data_index * old_component.type_size], &new_component.component_data[new_data_index * new_component.type_size]);
                break;
            }

    previous_variant->remove_actor(actor);

    actor->variant    = this;
    actor->data_index = static_cast<uint32_t>(new_data_index);
}

ComponentDataType* ActorVariant::get_last_element_memory(ComponentTypeID type_id)
{
    for (auto& component : components)
    {
        if (component.type_id == type_id)
        {
            return &component.component_data[(linked_actors.size() - 1) * component.type_size];
        }
    }
    LOG_FATAL("failed to find component type with the given type_id");
}
} // namespace ecs