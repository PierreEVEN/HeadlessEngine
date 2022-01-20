#pragma once

#include "ecs.h"
#include "ecs_type.h"

#include <vector>

namespace ecs
{
class Actor final
{
  public:
    ~Actor();

    template <typename Component_T, typename... CtorArgs_T> Component_T* add_component(CtorArgs_T&&... ctor_args)
    {
        return context->add_component<Component_T, CtorArgs_T...>(actor_id, std::forward<CtorArgs_T>(ctor_args)...);
    }

    template <typename Component_T> Component_T* get_component()
    {
        return context->get_component<Component_T>(actor_id);
    }

    template <typename Component_T> void remove_component()
    {
        context->remove_component<Component_T>(actor_id);
    }

    // Move this actor to another ECS context
    void move_to(ECS* new_context);

    // Make a copy of this actor (copy components too)
    [[nodiscard]] std::shared_ptr<Actor> duplicate() const;

  private:
    friend class ECS;
    Actor(ECS* context, const ActorID& new_id);

    const ActorID actor_id;
    ECS*          context;
};
} // namespace ecs