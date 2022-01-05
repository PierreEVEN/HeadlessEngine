#pragma once

#include "gfx/command_buffer.h"

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
    void draw_mesh(Mesh* in_buffer, MaterialInstance* in_material, RenderLayer render_layer) override;
    void draw_mesh_indirect(Mesh* in_buffer, MaterialInstance* in_material, RenderLayer render_layer) override;
    void draw_mesh_instanced(Mesh* in_buffer, MaterialInstance* in_material, RenderLayer render_layer) override;
    void draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInstance* in_material, RenderLayer render_layer) override;

    VkCommandBuffer& operator*()
    {
        return *command_buffer;
    }

private:
    SwapchainImageResource<VkCommandBuffer> command_buffer;
};

} // namespace gfx::vulkan