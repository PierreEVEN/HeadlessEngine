#pragma once

#include <vulkan/vulkan.hpp>
#include "vulkan/vk_unit.h"
#include "gfx/render_pass_instance.h"

namespace gfx::vulkan
{
class RenderPassInstance_VK final : public RenderPassInstance
{
public:
    RenderPassInstance_VK(uint32_t width, uint32_t height, RenderPass* base, const std::optional<std::vector<std::shared_ptr<Texture>>>& images);

    ~RenderPassInstance_VK() override;

private:

    
    void                                begin(CommandBuffer* command_buffer);
  void                                  end(CommandBuffer* command_buffer);
public:
    void resize(uint32_t width, uint32_t height) override;
private:
    uint32_t                              framebuffer_width;
    uint32_t                              framebuffer_height;
    SwapchainImageResource<VkFramebuffer> framebuffers;
};
} // namespace gfx::vulkan
