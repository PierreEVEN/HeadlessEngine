#pragma once

#include "gfx/command_buffer.h"
#include "gfx/render_pass_reference.h"

#include "vulkan/vk_unit.h"

#include "vulkan/vulkan.hpp"

namespace gfx::vulkan
{

class CommandBuffer_VK : public CommandBuffer
{
  public:
    CommandBuffer_VK(const std::string& name);
    virtual ~CommandBuffer_VK() override;

    void draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) override;
    void draw_mesh(StaticMesh* in_buffer, MaterialInstance* in_material, uint32_t instance_count = 1, uint32_t first_instance = 0) override;
    void draw_mesh(StaticMesh* in_buffer, MaterialInstance* in_material, uint32_t first_index, uint32_t vertex_offset, uint32_t index_count, uint32_t instance_count = 1, uint32_t first_instance = 0) override;
    void draw_mesh_indirect(StaticMesh* in_buffer, MaterialInstance* in_material) override;
    void set_scissor(const Scissor& scissors) override;
    void push_constant(bool is_vertex_buffer, const MaterialInstance* material, const void* data, uint32_t data_size) override;

    VkCommandBuffer& operator*()
    {
        return *command_buffer;
    }

    void start() override;
    void end() override;

  private:
    SwapchainImageResource<VkCommandBuffer> command_buffer;
    void                                    bind_material(MaterialInstance* in_material);

  public:
};

} // namespace gfx::vulkan