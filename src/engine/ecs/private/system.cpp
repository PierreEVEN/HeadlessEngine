#include "ecs/system.h"

#include "ecs/ecs.h"

namespace ecs
{

void SystemFactory::execute_tick() const
{
    for (auto& system_render : ticks)
    {
        const std::vector<ComponentTypeID>& key = system_render->get_key();
        for (ActorVariant* archetype : ECS::get().get_variants())
            if (std::ranges::includes(archetype->get_specification(), key))
                system_render->tick(archetype);
    }
}

} // namespace ecs
