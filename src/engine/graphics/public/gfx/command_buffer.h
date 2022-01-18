#pragma once

#include "render_pass_reference.h"

#include <cstdint>
#include <string>

namespace gfx
{
class Mesh;
class Texture;
class RenderPassID;
class Mesh;
class MaterialInstance;
class Buffer;

struct Scissor
{
    int32_t  offset_x;
    int32_t  offset_y;
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

    virtual void draw_mesh(Mesh* in_buffer, MaterialInstance* in_material, uint32_t instance_count = 1, uint32_t first_instance = 0)                                                                     = 0;
    virtual void draw_mesh(Mesh* in_buffer, MaterialInstance* in_material, uint32_t first_index, uint32_t vertex_offset, uint32_t index_count, uint32_t instance_count = 1, uint32_t first_instance = 0) = 0;
    virtual void draw_mesh_indirect(Mesh* in_buffer, MaterialInstance* in_material)                                                                                                                      = 0;
    virtual void draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex = 0, uint32_t instance_count = 1, uint32_t first_instance = 0)                                    = 0;
    virtual void set_scissor(const Scissor& scissors)                                                                                                                                                          = 0;
    virtual void push_constant(bool is_vertex_buffer, const MaterialInstance* material, const void* data, uint32_t data_size)                                                                                  = 0;
    template <typename Data_T> void push_constant(bool is_vertex_buffer, const MaterialInstance* material, const Data_T& data)
    {
        push_constant(is_vertex_buffer, material, &data, sizeof(Data_T));
    }

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