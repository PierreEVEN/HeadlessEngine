#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "gfx/surface.h"
#include "vk_device.h"
#include "vulkan/vk_unit.h"

namespace gfx::vulkan
{
class FenceResource_VK final
{
  public:
    struct CI_Fence
    {
    };

    FenceResource_VK(const std::string& name, const CI_Fence& create_infos);
    ~FenceResource_VK();

  private:
    VkFence fence = VK_NULL_HANDLE;
};

class SemaphoreResource_VK final
{
  public:
    struct CI_Semaphore
    {
    };
    SemaphoreResource_VK(const std::string& name, const CI_Semaphore& create_infos);
    ~SemaphoreResource_VK();

  private:
    VkSemaphore semaphore = VK_NULL_HANDLE;
};

class SwapchainResource_VK final
{
  public:
    struct CI_Swapchain
    {
    };

    SwapchainResource_VK(const std::string& name, const CI_Swapchain& create_infos);
    ~SwapchainResource_VK();

  private:
    VkSemaphore semaphore = VK_NULL_HANDLE;
};
class SurfaceResource_VK final
{
  public:
    struct CI_Surface
    {
    };

    SurfaceResource_VK(const std::string& name, const CI_Surface& create_infos);
    ~SurfaceResource_VK();

  private:
    VkSemaphore semaphore = VK_NULL_HANDLE;
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
        return surface_format;
    }

    std::shared_ptr<Texture> get_surface_render_texture() const
    {
        return surface_texture;
    }

  private:
    TGpuHandle<SurfaceResource_VK>   surface;
    TGpuHandle<SwapchainResource_VK> swapchain;
    TGpuHandle<QueueResource_VK>     present_queue;

    VkSurfaceFormatKHR            surface_format;
    VkPresentModeKHR              present_mode;
    VkCompositeAlphaFlagBitsKHR   composite_alpha;
    VkSurfaceTransformFlagBitsKHR transform_flags;
    uint32_t                      present_queue_family;
    void                          recreate_swapchain();

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
