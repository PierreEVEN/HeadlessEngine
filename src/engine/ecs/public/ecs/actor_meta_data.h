#pragma once
#include "component.h"

#include <vector>

namespace ecs
{
struct ActorMetaData;

struct ActorVariant
{
    struct ComponentData
    {
        std::size_t                    data_size;
        std::size_t                    type_size;
        std::vector<ComponentDataType> component_data;
        ComponentTypeID                type_id;
        IComponent*                    component_type;
    };

    ActorVariant(const std::vector<ComponentTypeID>& in_specification);

    void add_actor(ActorMetaData* actor);
    void remove_actor(ActorMetaData* actor);
    void move_actor_from(ActorMetaData* actor, ActorVariant* previous_variant);

    [[nodiscard]] ComponentDataType* get_last_element_memory(ComponentTypeID type_id);

    [[nodiscard]] std::vector<ComponentTypeID> get_specification() const
    {
        std::vector<ComponentTypeID> spec;
        spec.resize(components.size());
        for (uint32_t i = 0; i < components.size(); ++i)
        {
            spec[i] = components[i].type_id;
        }
        std::ranges::sort(spec);
        return spec;
    }

    std::vector<ComponentData> components;
    std::vector<ActorID>       linked_actors;

  private:
    void resize_component_data(ComponentData& component) const
    {
        component.component_data.resize(linked_actors.size() * component.type_size);
        /**
        const size_t desired_size = linked_actors.size() * component.type_size;
        if (desired_size > component.data_size)
        {
            component.data_size *= 2;
            component.data_size += component.type_size;
            auto* new_data = new ComponentDataType[component.data_size];
            delete[] component.component_data;
            component.component_data = new_data;
        }
        else if (desired_size <= component.data_size / 2)
        {
            component.data_size /= 2;
            auto* new_data = new ComponentDataType[component.data_size];
            delete[] component.component_data;
            component.component_data = new_data;
        }
        */
    }
};

struct ActorMetaData
{
    ActorVariant* variant;
    uint32_t      data_index;
    ActorID       actor_id;
};
} // namespace ecs