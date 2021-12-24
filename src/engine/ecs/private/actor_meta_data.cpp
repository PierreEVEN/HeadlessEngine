#include "ecs/actor_meta_data.h"

#include "ecs/ecs.h"

namespace ecs
{
ActorVariant::ActorVariant(const std::vector<ComponentTypeID>& in_specification)
{
    components.resize(in_specification.size());
    for (size_t i = 0; i < in_specification.size(); ++i)
    {
        IComponent* component = ECS::get().get_component_type(in_specification[i]);
        if (!component)
            LOG_FATAL("component with id %lu is not registered", in_specification[i])

        components[i] = ComponentData{
            .data_size      = 0,
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
        resize_component_data(component);
}

void ActorVariant::remove_actor(ActorMetaData* actor)
{
    size_t removed_index = 0;
    for (size_t i = 0; i < linked_actors.size(); ++i)
    {
        if (linked_actors[i] == actor->actor_id)
        {
            removed_index = i;
            break;
        }
    }

    for (auto& component : components)
        component.component_type->component_move(&component.component_data[(removed_index)*component.type_size], &component.component_data[(linked_actors.size() - 1) * component.type_size]);
    
    ECS::get().actor_registry[*(linked_actors.end() - 1)].data_index = static_cast<uint32_t>(removed_index);

    linked_actors.erase(linked_actors.end() - 1);

    for (auto& component : components)
        resize_component_data(component);

    actor->variant    = nullptr;
    actor->data_index = 0;
}

void ActorVariant::move_actor_from(ActorMetaData* actor, ActorVariant* previous_variant)
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