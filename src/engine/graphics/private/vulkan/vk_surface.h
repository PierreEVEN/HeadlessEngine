#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "gfx/resource/gpu_resource.h"
#include "gfx/surface.h"
#include "vk_device.h"
#include "vulkan/vk_unit.h"

namespace gfx::vulkan
{
class SurfaceResource_VK final
{
  public:
    struct CI_Surface
    {
        application::window::Window* container;
    };

    SurfaceResource_VK(const std::string& name, const CI_Surface& create_infos);
    ~SurfaceResource_VK();

    TGpuHandle<QueueResource_VK> present_queue;
    uint32_t                     present_queue_family;
    VkSurfaceFormatKHR           surface_format;
    VkCompositeAlphaFlagBitsKHR  composite_alpha;
    VkSurfaceKHR                 surface = VK_NULL_HANDLE;
    const CI_Surface             parameters;
};

class SwapchainResource_VK final
{
  public:
    struct CI_Swapchain
    {
        TGpuHandle<SurfaceResource_VK> surface;
        TGpuHandle<SwapchainResource_VK> previous_swapchain;
    };

    SwapchainResource_VK(const std::string& name, const CI_Swapchain& create_infos);
    ~SwapchainResource_VK();

    [[nodiscard]] SwapchainImageResource<VkImage>& get_swapchain_images()
    {
        return swapchain_images;
    }

    VkSwapchainKHR                swapchain = VK_NULL_HANDLE;
    VkPresentModeKHR              present_mode;
    VkCompositeAlphaFlagBitsKHR   composite_alpha;
    VkSurfaceTransformFlagBitsKHR transform_flags;
    const CI_Swapchain            parameters;

  private:
    SwapchainImageResource<VkImage> swapchain_images;
};

class Surface_VK : public Surface
{
  public:
    Surface_VK(application::window::Window* container);
    virtual ~Surface_VK();

    void render() override;

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

    std::shared_ptr<Texture>          surface_texture;
    SwapchainImageResource<ImageData> swapchain_resources;
    application::window::Window*      window_container = nullptr;
};
} // namespace gfx::vulkan
