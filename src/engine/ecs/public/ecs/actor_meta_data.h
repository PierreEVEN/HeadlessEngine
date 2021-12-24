#pragma once
#include "component.h"

#include <vector>

namespace ecs
{
struct ActorVariant
{
    std::vector<ComponentTypeID> variant_specification;

    std::vector<ComponentDataType*> component_data;
    std::vector<ActorID>            linked_actors;
    std::vector<std::size_t>        per_component_data_size;

    ComponentDataType* add_actor(ActorID id);
    void remove_actor(ActorID id);
    void move_actor_from(ActorID, ActorVariant* previous_variant);
};

inline ComponentDataType* ActorVariant::add_actor(ActorID id)
{
}

inline void ActorVariant::remove_actor(ActorID id)
{
}

inline void ActorVariant::move_actor_from(ActorID, ActorVariant* previous_variant)
{
}

struct ActorMetaData
{
    ActorVariant* variant;
    std::size_t   data_index;
};
} // namespace ecs