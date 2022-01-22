#include "ecs/ecs.h"

#include "ecs/actor.h"

namespace ecs
{

static ActorID              last_actor_id = 0;
static std::vector<ActorID> actor_id_pool = {};

ActorID ECS::make_new_actor_id()
{
    if (!actor_id_pool.empty())
    {
        const ActorID last = *(actor_id_pool.end() - 1);
        actor_id_pool.pop_back();
        return last;
    }
    return last_actor_id++;
}

void ECS::free_actor_id(const ActorID actor)
{
    actor_id_pool.emplace_back(actor);
}

ActorVariant* ECS::find_variant(std::vector<ComponentTypeID>& variant_spec)
{
    std::ranges::sort(variant_spec);
    for (ActorVariant* archetype : variant_registry)
        if (std::ranges::equal(variant_spec, archetype->get_specification()))
            return archetype;

    auto* new_variant = new ActorVariant(this, variant_spec);
    variant_registry.emplace_back(new_variant);
    return new_variant;
}

ECS::~ECS()
{
    for (const auto& variant : variant_registry)
        delete variant;
}

void ECS::init_actor(void* actor_memory)
{
    Actor* actor    = static_cast<Actor*>(actor_memory);
    actor->actor_id = make_new_actor_id();
    actor->context  = this;
    register_actor(actor->actor_id);
}

void ECS::register_actor(const ActorID& actor)
{
    actor_registry.emplace(actor, ActorMetaData{
                                      .variant    = nullptr,
                                      .data_index = 0,
                                      .actor_id   = actor,
                                  });
}

void ECS::remove_actor(const ActorID& removed_actor)
{
    auto actor_it = actor_registry.find(removed_actor);

    if (actor_it == actor_registry.end())
        return; // it doesn't exist

    robin_hood::pair<ActorID, ActorMetaData>& pair       = *actor_it;
    const ActorMetaData&                      actor_data = pair.second;

    if (actor_data.variant)
        actor_data.variant->remove_actor(&pair.second, false);

    actor_registry.erase(actor_it);
}

void ECS::move_actor(const ActorID& actor, ECS* old_context, ECS* new_context)
{
    ActorMetaData actor_data = old_context->actor_registry[actor];

    auto  moved_actor_specification = actor_data.variant->get_specification();
    auto* old_context_variant       = old_context->find_variant(moved_actor_specification);
    auto* new_context_variant       = new_context->find_variant(moved_actor_specification);

    ActorVariant::move_actor_to_variant(&actor_data, old_context_variant, new_context_variant);

    new_context->actor_registry.emplace(actor, actor_data);
    old_context->actor_registry.erase(actor);
}

ActorID ECS::duplicate_actor(ActorID actor)
{
    const ActorMetaData& current_actor_data = actor_registry[actor];

    ActorMetaData new_actor_infos{.actor_id = make_new_actor_id()};
    current_actor_data.variant->duplicate_actor(&current_actor_data, &new_actor_infos);

    actor_registry.emplace(new_actor_infos.actor_id, new_actor_infos);
    return new_actor_infos.actor_id;
}

void ECS::tick()
{
    for (const auto& variant : get_variants())
        for (size_t i = 0; i < variant->components.size(); ++i)
            if (variant->components[i].component_type->tick_runner) // Only if the component implement the tick method
                variant->components[i].component_type->tick_runner->execute(variant->components[i].component_data_buffer.data(), variant->linked_actors.size());
    system_factory.execute_tick(this);
    on_tick.execute();
}

void ECS::pre_render(gfx::View* view)
{
    on_pre_render.execute(view);
    system_factory.execute_pre_render(this, view);
    for (const auto& variant : get_variants())
        for (size_t i = 0; i < variant->components.size(); ++i)
            if (variant->components[i].component_type->pre_render_runner) // Only if the component implement the pre_render method
                variant->components[i].component_type->pre_render_runner->execute(variant->components[i].component_data_buffer.data(), variant->linked_actors.size(), view);
}

void ECS::render(gfx::View* view, gfx::CommandBuffer* command_buffer)
{
    for (const auto& variant : get_variants())
        for (size_t i = 0; i < variant->components.size(); ++i)
            if (variant->components[i].component_type->render_runner) // Only if the component implement the render method
                variant->components[i].component_type->render_runner->execute(variant->components[i].component_data_buffer.data(), variant->linked_actors.size(), view, command_buffer);
    system_factory.execute_render(this, view, command_buffer);
    on_render.execute(view, command_buffer);
}
} // namespace ecs
