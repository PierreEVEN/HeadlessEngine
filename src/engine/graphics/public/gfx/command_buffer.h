#pragma once

#include "types/bound.h"

#include <cstdint>

namespace gfx
{
class Mesh;
class MaterialInterface;

class CommandBuffer
{
    using RenderLayer = int64_t;

  public:
    virtual void draw_mesh(Mesh* in_buffer, MaterialInterface* in_material, const Bound& bounds, RenderLayer render_layer)                    = 0;
    virtual void draw_mesh_indirect(Mesh* in_buffer, MaterialInterface* in_material, const Bound& bounds, RenderLayer render_layer)           = 0;
    virtual void draw_mesh_instanced(Mesh* in_buffer, MaterialInterface* in_material, const Bound& bounds, RenderLayer render_layer)          = 0;
    virtual void draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInterface* in_material, const Bound& bounds, RenderLayer render_layer) = 0;

  private:
};

} // namespace gfx