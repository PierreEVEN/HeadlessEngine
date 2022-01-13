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
    void draw_mesh(StaticMesh* in_buffer, MaterialInstance* in_material) override;
    void draw_mesh_indirect(StaticMesh* in_buffer, MaterialInstance* in_material) override;
    void draw_mesh_instanced(StaticMesh* in_buffer, MaterialInstance* in_material) override;
    void draw_mesh_instanced_indirect(StaticMesh* in_buffer, MaterialInstance* in_material) override;
    void set_scissor(const Scissor& scissors) override;

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