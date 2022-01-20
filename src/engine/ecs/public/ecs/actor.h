#pragma once

#include "ecs.h"
#include "ecs_type.h"

#include <vector>

namespace ecs
{

class Actor;

template <typename Component_T> class ComponentReference
{
  public:
    ComponentReference(Actor* owner) : owning_actor(owner)
    {
    }

    Component_T* operator->() const;

    Component_T& operator*();

  private:
    Actor* owning_actor;
};

class Actor final
{
  public:
    ~Actor();

    template <typename Component_T, typename... CtorArgs_T> ComponentReference<Component_T> add_component(CtorArgs_T&&... ctor_args)
    {
        context->add_component<Component_T, CtorArgs_T...>(actor_id, std::forward<CtorArgs_T>(ctor_args)...);
        return ComponentReference<Component_T>(this);
    }

    template <typename Component_T> ComponentReference<Component_T> get_component()
    {
        return ComponentReference<Component_T>(this);
    }

    template <typename Component_T> void remove_component() const
    {
        context->remove_component<Component_T>(actor_id);
    }

    // Move this actor to another ECS context
    void move_to(ECS* new_context);

    // Make a copy of this actor (copy components too)
    [[nodiscard]] std::shared_ptr<Actor> duplicate() const;

  private:
    template<typename Component_T>
    friend class ComponentReference;
    friend class ECS;
    Actor(ECS* context, const ActorID& new_id);

    const ActorID actor_id;
    ECS*          context;
};

template <typename Component_T> Component_T* ComponentReference<Component_T>::operator->() const
{
    return owning_actor->context->get_component<Component_T>(owning_actor->actor_id);
}

template <typename Component_T> Component_T& ComponentReference<Component_T>::operator*()
{
    return *owning_actor->context->get_component<Component_T>(owning_actor->actor_id);
}
} // namespace ecs