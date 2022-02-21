#pragma once

#include "ecs/object.h"

#include <cstdint>
#include <cpputils/logger.hpp>
#include <types/no_copy.h>

class Ecs_New;
using ObjectID = uint64_t;
#define NULL_ID 0


class ObjectPtr final : public NoCopy
{
    friend class Object;
    friend class Family;

  public:
    ObjectPtr(Ecs_New* in_ecs) : ecs(in_ecs)
    {
    }

    void move_to(Ecs_New*)
    {
        LOG_FATAL("NIY");
    }

    Component<ComponentBase> get_component(Class* component_class);
    Component<ComponentBase> add_component(Class* component_class);
    void                    remove_component(Class* component_class);

  private:
    Ecs_New* ecs;
    uint32_t ref_counter = 0;

    uint32_t pool_index    = 0;
    Family*  object_family = nullptr;
    ObjectID object_id     = NULL_ID;
};
