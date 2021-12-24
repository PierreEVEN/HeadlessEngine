#include "ecs/ecs.h"

#include "ecs/actor.h"

namespace ecs
{
std::unique_ptr<ECS> ecs_singleton;

ECS& ECS::get()
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

void ECS::add_empty_actor(const ActorID actor_instance_id)
{
    actor_registry.emplace(actor_instance_id, ActorMetaData{
        .variant    = nullptr,
        .data_index = 0,
        .actor_id   = actor_instance_id,
    });
}

void ECS::remove_actor(const ActorID& removed_actor)
{
    auto actor_it = actor_registry.find(removed_actor);

    if (actor_it == actor_registry.end())
        return; // it doesn't exist

    robin_hood::pair<ActorID, ActorMetaData>& pair = *actor_it;
    const ActorMetaData& actor_data = pair.second;

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
    // Execute tick, pre-render, render method for every components
    for (const auto& variant : ECS::get().get_variants())
    {
        for (size_t i = 0; i < variant->components.size(); ++i)
        {
            const IComponent* component_type = variant->components[i].component_type;
            if (component_type->tick_runner) // Only if the component implement the tick method
                component_type->tick_runner->execute(variant->components[i].component_data.data(), variant->linked_actors.size());
        }
    }
    system_factory.execute_tick();
}

void ECS::pre_render()
{
    // Execute tick, pre-render, render method for every components
    for (const auto& variant : ECS::get().get_variants())
    {
        for (size_t i = 0; i < variant->components.size(); ++i)
        {
            const IComponent* component_type = variant->components[i].component_type;
            if (component_type->pre_render_runner) // Only if the component implement the tick method
                component_type->pre_render_runner->execute(variant->components[i].component_data.data(), variant->linked_actors.size());
        }
    }
}

void ECS::render(gfx::CommandBuffer* command_buffer)
{
    // Execute tick, pre-render, render method for every components
    for (const auto& variant : ECS::get().get_variants())
    {
        for (size_t i = 0; i < variant->components.size(); ++i)
        {
            const IComponent* component_type = variant->components[i].component_type;
            if (component_type->render_runner) // Only if the component implement the tick method
                component_type->render_runner->execute(variant->components[i].component_data.data(), variant->linked_actors.size(), command_buffer);
        }
    }
}
} // namespace ecs
