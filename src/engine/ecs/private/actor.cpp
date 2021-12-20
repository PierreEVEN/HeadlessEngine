

#include "ecs/actor.h"

ecs::Actor::Actor()
{
    actor_id = ECS::get().make_new_actor_id();
    ECS::get().add_empty_actor(actor_id);
}

ecs::Actor::~Actor()
{
    ECS::get().remove_actor(actor_id);
}
