#pragma once

#include "render_pass_reference.h"

#include <cstdint>
#include <string>

namespace gfx
{
class Texture;
class RenderPassID;
class StaticMesh;
class MaterialInstance;
class Buffer;

struct Scissor
{
    int32_t offset_x;
    int32_t offset_y;
    uint32_t width;
    uint32_t height;
};

class CommandBuffer
{
    friend class RenderPassInstance;

  public:
    static CommandBuffer* create(const std::string& name);

    virtual ~CommandBuffer() = default;

    using RenderLayer = int64_t;

    virtual void draw_mesh(StaticMesh* in_buffer, MaterialInstance* in_material)                                                                                      = 0;
    virtual void draw_mesh_indirect(StaticMesh* in_buffer, MaterialInstance* in_material)                                                                             = 0;
    virtual void draw_mesh_instanced(StaticMesh* in_buffer, MaterialInstance* in_material)                                                                            = 0;
    virtual void draw_mesh_instanced_indirect(StaticMesh* in_buffer, MaterialInstance* in_material)                                                                   = 0;
    virtual void draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) = 0;
    virtual void set_scissor(const Scissor& scissors)                                                                                                           = 0;

    [[nodiscard]] const RenderPassID& get_render_pass() const
    {
        return *render_pass;
    }

    virtual void start() = 0;
    virtual void end()   = 0;

  protected:
    const RenderPassID* render_pass;
};

} // namespace gfx