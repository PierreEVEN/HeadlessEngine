#pragma once
#include "ecs/ecs.h"

#include <types/bound.h>

namespace gfx
{
class StaticMeshComponent
{
  public:

    static void add_systems(ecs::SystemFactory* factory);

    Bound& get_bounds()
    {
        return bounds;
    }

  private:
    Bound bounds;
};
} // namespace sm

// @TODO : register component
