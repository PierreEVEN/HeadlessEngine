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

class Actor
{
  public:
    virtual ~Actor();

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
    template <typename Actor_T = Actor> [[nodiscard]] std::shared_ptr<Actor_T> duplicate() const
    {
        Actor_T* new_actor = static_cast<Actor_T*>(malloc(sizeof(Actor_T)));
        memcpy(new_actor, this, sizeof(Actor_T));
        new_actor->actor_id = context->duplicate_actor(actor_id);
        return std::shared_ptr<Actor_T>(new_actor);
    }

  protected:
    Actor() = default;

  private:
    template <typename Component_T> friend class ComponentReference;
    friend class ECS;
    ActorID actor_id;
    ECS*    context;
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