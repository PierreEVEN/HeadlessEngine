#include "ecs/SystemTick.h"

#include "ecs/ecs.h"

namespace ecs
{

void SystemFactory::execute_tick() const
{
    for (auto& system_render : ticks)
    {
        const std::vector<ComponentTypeID>& key = system_render->get_key();
        for (ActorVariant* archetype : singleton().get_variants())
            if (std::ranges::includes(archetype->get_specification(), key))
                system_render->tick(archetype);
    }
}

void SystemFactory::execute_pre_render(gfx::View* view) const
{
    for (auto& system_render : pre_renders)
    {
        const std::vector<ComponentTypeID>& key = system_render->get_key();
        for (ActorVariant* archetype : singleton().get_variants())
            if (std::ranges::includes(archetype->get_specification(), key))
                system_render->pre_render(archetype, view);
    }
}

void SystemFactory::execute_render(gfx::View* view) const
{
    for (auto& system_render : renders)
    {
        const std::vector<ComponentTypeID>& key = system_render->get_key();
        for (ActorVariant* archetype : singleton().get_variants())
            if (std::ranges::includes(archetype->get_specification(), key))
                system_render->render(archetype, view);
    }
}
} // namespace ecs
