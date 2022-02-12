#pragma once

#include "gfx/render_pass_instance.h"
#include "vk_device.h"
#include "vk_render_pass.h"
#include "vk_texture.h"
#include "vulkan/vk_unit.h"
#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class RenderPassInstance_VK final : public RenderPassInstance
{
    friend class Surface_VK;

  public:
    RenderPassInstance_VK(uint32_t width, uint32_t height, const RenderPassID& base, const std::vector<std::shared_ptr<Texture>>& images);
    ~RenderPassInstance_VK() override = default;

    void resize(uint32_t width, uint32_t height, const std::vector<std::shared_ptr<Texture>>& surface_texture) override;

  protected:
    void begin_pass() override;
    void submit() override;

  private:
    struct FrameData
    {
        TGpuHandle<FramebufferResource_VK> framebuffer;
        TGpuHandle<SemaphoreResource_VK>   render_finished_semaphore;
        TGpuHandle<FenceResource_VK>       render_finished_fences;
    };

    SwapchainImageResource<FrameData> frame_data                        = {};
    TGpuHandle<SemaphoreResource_VK>  swapchain_image_acquire_semaphore = {};
    TGpuHandle<RenderPassResource_VK> render_pass;
    std::string                pass_name;
};
} // namespace gfx::vulkan