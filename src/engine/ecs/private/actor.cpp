

#include "ecs/actor.h"

ecs::Actor::Actor(ECS* new_context, const ActorID& new_id) : actor_id(new_id), context(new_context)
{
    context->register_actor(actor_id);
}

ecs::Actor::~Actor()
{
    context->remove_actor(actor_id);
}

void ecs::Actor::move_to(ECS* new_context)
{
    ECS::move_actor(actor_id, context, new_context);
    context = new_context;
}

std::shared_ptr<ecs::Actor> ecs::Actor::duplicate() const
{
    return std::make_shared<Actor>(context, context->duplicate_actor(actor_id));
}
