#pragma once

#include "ecs.h"
#include "ecs_type.h"

#include <vector>

namespace ecs
{
class Actor final
{
  public:
    Actor(const ActorID& id);
    ~Actor();

    template <typename Component_T, typename... CtorArgs_T> Component_T* add_component(CtorArgs_T&&... ctor_args)
    {
        return singleton().add_component<Component_T, CtorArgs_T...>(actor_id, std::forward<CtorArgs_T>(ctor_args)...);
    }

    template <typename Component_T> void remove_component()
    {
        singleton().remove_component<Component_T>(actor_id);
    }

    void move_to(ECS* target) {}

  private:
    ActorID actor_id;
};
} // namespace ecs