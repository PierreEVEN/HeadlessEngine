#pragma once

#include "gfx/command_buffer.h"

#include "unit.h"
#include "vulkan/vulkan.hpp"

namespace gfx::vulkan
{

class CommandBuffer_VK : public CommandBuffer
{
  public:
    CommandBuffer_VK();
    virtual ~CommandBuffer_VK() override;

    void draw_mesh(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer) override;
    void draw_mesh_indirect(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer) override;
    void draw_mesh_instanced(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer) override;
    void draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer) override;

    VkCommandBuffer& operator*()
    {
        return *command_buffer;
    }

private:
    SwapchainImageResource<VkCommandBuffer> command_buffer;
};

} // namespace gfx::vulkan