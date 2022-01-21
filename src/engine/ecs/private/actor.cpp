

#include "ecs/actor.h"


ecs::Actor::~Actor()
{
    context->remove_actor(actor_id);
    ECS::free_actor_id(actor_id);
}

void ecs::Actor::move_to(ECS* new_context)
{
    ECS::move_actor(actor_id, context, new_context);
    context = new_context;
}

