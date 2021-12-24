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
    ActorMetaData actor_data;
    actor_data.variant                = nullptr;
    actor_data.data_index             = 0;
    actor_data.actor_id               = actor_instance_id;
    actor_registry[actor_instance_id] = actor_data;
}

void ECS::remove_actor(const ActorID& removed_actor)
{
    if (!actor_registry.contains(removed_actor))
        return; // it doesn't exist

    ActorMetaData& record = actor_registry[removed_actor];

    ActorVariant* oldArchetype = record.variant;

    if (!oldArchetype)
    {
        actor_registry.erase(removed_actor);
        return; // we wouldn't know where to delete
    }

    oldArchetype->remove_actor(&actor_registry[removed_actor]);
}

ActorID ECS::make_new_actor_id()
{
    return last_actor_id++;
}

void ECS::tick()
{
    auto now = std::chrono::steady_clock::now();
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
    LOG_DEBUG("ECS : iterate over components using a callback (Should be close to 2-b): \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

    now = std::chrono::steady_clock::now();
    system_factory.execute_tick();
    LOG_DEBUG("ECS : iterate over components using a System (Should be close to 2-c) : \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());
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
