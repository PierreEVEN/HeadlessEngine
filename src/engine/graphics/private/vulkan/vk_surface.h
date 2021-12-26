#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include "gfx/surface.h"

#include "unit.h"

namespace gfx::vulkan
{
class Surface_VK : public Surface
{
  public:
    Surface_VK(application::window::Window* container);
    virtual ~Surface_VK();

    void display(RenderTarget& render_target) override;

  private:
    VkSurfaceKHR   surface   = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    VkQueue                       present_queue = VK_NULL_HANDLE;
    VkSurfaceFormatKHR            surface_format;
    VkPresentModeKHR              present_mode;
    VkCompositeAlphaFlagBitsKHR   composite_alpha;
    VkSurfaceTransformFlagBitsKHR transform_flags;
    uint32_t                      present_queue_family;
    void                          recreate_swapchain();

    struct ImageData
    {
        VkCommandBuffer command_buffer             = VK_NULL_HANDLE;
        VkFence         image_in_flight            = VK_NULL_HANDLE;
        VkSemaphore     image_acquire_semaphore    = VK_NULL_HANDLE;
        VkSemaphore     render_finnished_semaphore = VK_NULL_HANDLE;
        VkFence         in_flight_fence            = VK_NULL_HANDLE;
    };

    SwapchainImageResource<ImageData> swapchain_resources;
    application::window::Window*      window_container = nullptr;
};
} // namespace gfx::vulkan
