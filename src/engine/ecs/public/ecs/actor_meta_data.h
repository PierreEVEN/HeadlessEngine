#pragma once
#include "ecs/component.h"

#include <vector>

namespace ecs
{
struct ActorMetaData
{
    class ActorVariant* variant    = nullptr;
    uint32_t            data_index = 0;
    ActorID             actor_id;
};

class ActorVariant
{
  public:
    struct ComponentData
    {
        std::size_t                    type_size;
        std::vector<ComponentDataType> component_data;
        ComponentTypeID                type_id;
        IComponentHelper*              component_type;
    };

    ActorVariant(const std::vector<ComponentTypeID>& in_specification);

    void add_actor(ActorMetaData* actor);
    void remove_actor(ActorMetaData* actor);
    void copy_to_this_variant(ActorMetaData* actor, ActorVariant* previous_variant);

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
} // namespace ecs