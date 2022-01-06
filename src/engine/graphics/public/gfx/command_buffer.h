#pragma once

#include <cstdint>
#include <string>

namespace gfx
{
class Mesh;
class MaterialInstance;

class CommandBuffer
{
  public:
    virtual ~CommandBuffer() = default;

    using RenderLayer = int64_t;

    virtual void draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) = 0;

    virtual void draw_mesh(Mesh* in_buffer, MaterialInstance* in_material)                     = 0;
    virtual void draw_mesh_indirect(Mesh* in_buffer, MaterialInstance* in_material)            = 0;
    virtual void draw_mesh_instanced(Mesh* in_buffer, MaterialInstance* in_material)           = 0;
    virtual void draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInstance* in_material)  = 0;

    std::string render_pass;
};

} // namespace gfx