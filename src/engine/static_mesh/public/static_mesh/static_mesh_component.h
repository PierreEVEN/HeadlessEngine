#pragma once
#include "ecs/ecs.h"
#include "gfx/material_instance.h"
#include "gfx/mesh.h"

#include <types/bound.h>

namespace gfx
{
class StaticMeshComponent
{
  public:
    static void add_systems(ecs::SystemFactory* factory);

    Bound& get_world_bounds()
    {
        return world_bounds;
    }

    static void init_system();
    static void init_system();
  private:
    std::shared_ptr<MaterialInstance> material;
    std::shared_ptr<Mesh>             mesh;
    Bound                             world_bounds;
};
} // namespace gfx

// @TODO : register component
