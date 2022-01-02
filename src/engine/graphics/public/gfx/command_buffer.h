#pragma once

#include <cstdint>

namespace gfx
{
class Mesh;
class MaterialInterface;

class CommandBuffer
{
public:
    virtual ~CommandBuffer() = default;

    using RenderLayer = int64_t;
    virtual void draw_mesh(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer) = 0;
    virtual void draw_mesh_indirect(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer) = 0;
    virtual void draw_mesh_instanced(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer) = 0;
    virtual void draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer) = 0;
};

} // namespace gfx