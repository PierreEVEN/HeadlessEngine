#pragma once

#include <vulkan/vulkan.h>

#include "gfx/resource/gpu_resource.h"
#include "gfx/surface.h"
#include "resources/vk_resource_surface.h"
#include "vk_device.h"
#include "vulkan/vk_unit.h"

namespace gfx::vulkan
{
class Surface_VK : public Surface
{
  public:
    Surface_VK(const std::string& name, application::window::Window* container);
    virtual ~Surface_VK();

    [[nodiscard]] bool prepare_next_frame() override;
    void               render() override;

    application::window::Window* get_container() const override
    {
        return window_container;
    }

    VkSurfaceFormatKHR get_surface_format() const
    {
        return surface->surface_format;
    }

    std::shared_ptr<Texture> get_surface_render_texture() const
    {
        return surface_texture;
    }

  private:
    TGpuHandle<SurfaceResource_VK>   surface;
    TGpuHandle<SwapchainResource_VK> swapchain;

    void recreate_swapchain();

    struct ImageData
    {
        TGpuHandle<FenceResource_VK>     image_in_flight;
        TGpuHandle<SemaphoreResource_VK> image_acquire_semaphore;
    };

    TGpuHandle<SemaphoreResource_VK>  current_image_acquire_semaphore;
    std::shared_ptr<Texture>          surface_texture;
    SwapchainImageResource<ImageData> swapchain_resources;
};
} // namespace gfx::vulkan
