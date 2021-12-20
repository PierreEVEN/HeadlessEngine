#pragma once

#include "ecs.h"
#include "ecs_type.h"

#include <vector>

namespace ecs
{
class Actor final
{
  public:
    Actor();
    ~Actor();

    template <typename Component_T, typename... CtorArgs_T> Component_T* add_component(CtorArgs_T&&... ctor_args)
    {
        return ECS::get().add_component<Component_T, CtorArgs_T...>(actor_id, std::forward<CtorArgs_T>(ctor_args)...);
    }

    template <typename Component_T> void remove_component()
    {
        ECS::get().remove_component<Component_T>(actor_id);
    }

  private:
    ActorID actor_id;
};
} // namespace ecs