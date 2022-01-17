#pragma once

#include "gfx/render_pass_instance.h"
#include "vulkan/vk_unit.h"
#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class RenderPassInstance_VK final : public RenderPassInstance
{
    friend class Surface_VK;

  public:
    RenderPassInstance_VK(uint32_t width, uint32_t height, const RenderPassID& base, const std::vector<std::shared_ptr<Texture>>& images);
    ~RenderPassInstance_VK() override;

    void resize(uint32_t width, uint32_t height, const std::vector<std::shared_ptr<Texture>>& surface_texture) override;

  protected:
    void begin_pass() override;
    void submit() override;

  private:
    SwapchainImageResource<VkFramebuffer> framebuffers                      = {};
    SwapchainImageResource<VkSemaphore>   render_finished_semaphore         = {};
    SwapchainImageResource<VkFence>       render_finished_fence             = {};
    VkSemaphore                           swapchain_image_acquire_semaphore = VK_NULL_HANDLE;
};
} // namespace gfx::vulkan
