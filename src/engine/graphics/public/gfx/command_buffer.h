#pragma once

#include "render_pass_reference.h"

#include <cstdint>
#include <string>

namespace gfx
{
class Texture;
class RenderPassID;
class Mesh;
class MaterialInstance;
class Buffer;

class CommandBuffer
{
    friend class RenderPassInstance;

  public:
    virtual ~CommandBuffer() = default;

    using RenderLayer = int64_t;

    virtual void draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) = 0;

    virtual void draw_mesh(Mesh* in_buffer, MaterialInstance* in_material)                    = 0;
    virtual void draw_mesh_indirect(Mesh* in_buffer, MaterialInstance* in_material)           = 0;
    virtual void draw_mesh_instanced(Mesh* in_buffer, MaterialInstance* in_material)          = 0;
    virtual void draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInstance* in_material) = 0;

    virtual void bind_buffer(std::string buffer_name, gfx::Buffer* in_buffer)   = 0;
    virtual void bind_texture(std::string buffer_name, gfx::Texture* in_buffer) = 0;

    const RenderPassID& get_render_pass() const
    {
        return render_pass;
    }

    virtual void start() = 0;
    virtual void end()   = 0;

  protected:
    RenderPassID render_pass;
};

} // namespace gfx