

#include "ecs/actor.h"

ecs::Actor::Actor(const ActorID& id)
    : actor_id(id)
{
}

ecs::Actor::~Actor()
{
    singleton().remove_actor(actor_id);
}
