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
};

struct ActorMetaData
{
    ActorVariant* variant;
    uint32_t      data_index;
    ActorID       actor_id;
};
} // namespace ecs