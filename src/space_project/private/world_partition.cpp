

#include "world_partition.h"

#include "application/application.h"

WorldPartition::WorldPartition() : velocity(0), aabb_center(0), aabb_extent(1)
{
}

void WorldPartition::include(WorldPartition& other)
{
}

void WorldPartition::render(const ViewPoint& view_point)
{
}

void WorldPartition::add_systems(ecs::SystemFactory* factory)
{
    factory->tick<WorldPartition>(
        []([[maybe_unused]] ecs::TSystemIterable<WorldPartition> iterator)
        {
            const double delta_time = application::get()->delta_time();

            /*
            // Move partitions
            for (const auto& [object, component] : iterator)
            {
                const auto& offset = component.velocity * delta_time;
                component.aabb_max += offset;
                component.aabb_min += offset;
            }
            
            // Check for collisions and merge or split partitions
            for (const auto& [object, component] : iterator)
            {
                const auto& offset = component.velocity * delta_time;
                component.aabb_max += offset;
                component.aabb_min += offset;
            }
            */
        });
}
