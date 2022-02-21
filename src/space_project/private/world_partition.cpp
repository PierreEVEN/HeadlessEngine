

#include "world_partition.h"

#include "application/application.h"

WorldPartition::WorldPartition() : velocity(0), aabb_center(0), aabb_extent(1)
{
}

void WorldPartition::include(WorldPartition& )
{
}

void WorldPartition::render(const ViewPoint& )
{
}
