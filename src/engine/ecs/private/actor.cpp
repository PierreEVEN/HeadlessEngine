

#include "ecs/actor.h"

ecs::Actor::Actor()
{
    actor_id = singleton().make_new_actor_id();
    singleton().add_empty_actor(actor_id);
}

ecs::Actor::~Actor()
{
    singleton().remove_actor(actor_id);
}
