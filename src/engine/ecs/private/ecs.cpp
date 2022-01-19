#include "ecs/ecs.h"

#include "ecs/actor.h"

namespace ecs
{
std::unique_ptr<ECS> ecs_singleton;

ECS& singleton()
{
    if (!ecs_singleton)
        ecs_singleton = std::unique_ptr<ECS>(new ECS());
    return *ecs_singleton;
}

ECS::~ECS()
{
}

ActorVariant* ECS::find_variant(std::vector<ComponentTypeID>& variant_spec)
{
    std::ranges::sort(variant_spec);
    for (ActorVariant* archetype : variant_registry)
        if (std::ranges::equal(variant_spec, archetype->get_specification()))
            return archetype;

    auto* new_variant = new ActorVariant(variant_spec);
    variant_registry.emplace_back(new_variant);
    return new_variant;
}

std::shared_ptr<Actor> ECS::make_empty_actor()
{
    const ActorID actor_id = make_new_actor_id();
    actor_registry.emplace(actor_id, ActorMetaData{
                                         .variant    = nullptr,
                                         .data_index = 0,
                                         .actor_id   = actor_id,
                                     });
    return std::make_shared<Actor>(actor_id);
}

void ECS::remove_actor(const ActorID& removed_actor)
{
    auto actor_it = actor_registry.find(removed_actor);

    if (actor_it == actor_registry.end())
        return; // it doesn't exist

    robin_hood::pair<ActorID, ActorMetaData>& pair       = *actor_it;
    const ActorMetaData&                      actor_data = pair.second;

    if (actor_data.variant)
        actor_data.variant->remove_actor(&pair.second);

    actor_registry.erase(actor_it);
}

ActorID ECS::make_new_actor_id()
{
    return last_actor_id++;
}

void ECS::tick()
{
    for (const auto& variant : get_variants())
        for (size_t i = 0; i < variant->components.size(); ++i)
            if (variant->components[i].component_type->tick_runner) // Only if the component implement the tick method
                variant->components[i].component_type->tick_runner->execute(variant->components[i].component_data.data(), variant->linked_actors.size());
    system_factory.execute_tick();
    on_tick.execute();
}

void ECS::pre_render(gfx::View* view)
{
    on_pre_render.execute(view);
    system_factory.execute_pre_render(view);
    for (const auto& variant : get_variants())
        for (size_t i = 0; i < variant->components.size(); ++i)
            if (variant->components[i].component_type->pre_render_runner) // Only if the component implement the pre_render method
                variant->components[i].component_type->pre_render_runner->execute(variant->components[i].component_data.data(), variant->linked_actors.size(), view);
}

void ECS::render(gfx::View* view)
{
    for (const auto& variant : get_variants())
        for (size_t i = 0; i < variant->components.size(); ++i)
            if (variant->components[i].component_type->render_runner) // Only if the component implement the render method
                variant->components[i].component_type->render_runner->execute(variant->components[i].component_data.data(), variant->linked_actors.size(), view);
    system_factory.execute_render(view);
    on_render.execute(view);
}
} // namespace ecs
