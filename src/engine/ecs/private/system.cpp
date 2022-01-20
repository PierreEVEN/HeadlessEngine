#include "ecs/system.h"

#include "ecs/ecs.h"

namespace ecs
{

void SystemFactory::execute_tick(ECS* context) const
{
    for (auto& system_render : ticks)
    {
        const std::vector<ComponentTypeID>& key = system_render->get_key();
        for (ActorVariant* archetype : context->get_variants())
            if (std::ranges::includes(archetype->get_specification(), key))
                system_render->tick(archetype);
    }
}

void SystemFactory::execute_pre_render(ECS* context, gfx::View* view) const
{
    for (auto& system_render : pre_renders)
    {
        const std::vector<ComponentTypeID>& key = system_render->get_key();
        for (ActorVariant* archetype : context->get_variants())
            if (std::ranges::includes(archetype->get_specification(), key))
                system_render->pre_render(archetype, view);
    }
}

void SystemFactory::execute_render(ECS* context, gfx::View* view, gfx::CommandBuffer* command_buffer) const
{
    for (auto& system_render : renders)
    {
        const std::vector<ComponentTypeID>& key = system_render->get_key();
        for (ActorVariant* archetype : context->get_variants())
            if (std::ranges::includes(archetype->get_specification(), key))
                system_render->render(archetype, view, command_buffer);
    }
}
} // namespace ecs
