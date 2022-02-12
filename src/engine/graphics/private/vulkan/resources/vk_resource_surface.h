#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "application/window.h"

#include <vulkan/vulkan.h>

#include "gfx/resource/gpu_resource.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_unit.h"

namespace application
{
namespace window
{
class Window;
}
} // namespace application

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

    VkSurfaceKHR surface;

    TGpuHandle<QueueResource_VK> present_queue;
    uint32_t                     present_queue_family;
    VkSurfaceFormatKHR           surface_format;
    VkCompositeAlphaFlagBitsKHR  composite_alpha;

    [[nodiscard]] uint32_t width() const
    {
        return container->absolute_width();
    }
    [[nodiscard]] uint32_t height() const
    {
        return container->absolute_height();
    }

  private:
    const application::window::Window* container;
};

class SwapchainResource_VK final
{
  public:
    struct CI_Swapchain
    {
        const TGpuHandle<SwapchainResource_VK> previous_swapchain;
        const TGpuHandle<SurfaceResource_VK>   surface;
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

  private:
    const TGpuHandle<SurfaceResource_VK> surface;
    SwapchainImageResource<VkImage>      swapchain_images;
};
} // namespace gfx::vulkan
