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
};

struct ActorMetaData
{
    ActorVariant* variant;
    std::size_t   data_index;
};
} // namespace ecs