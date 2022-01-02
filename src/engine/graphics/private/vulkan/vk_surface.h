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
        VkFence         image_in_flight            = VK_NULL_HANDLE;
        VkSemaphore     image_acquire_semaphore    = VK_NULL_HANDLE;
        VkSemaphore     render_finished_semaphore = VK_NULL_HANDLE;
        VkFence         in_flight_fence            = VK_NULL_HANDLE;
    };

    std::unique_ptr<CommandBuffer> main_command_buffer;
    std::shared_ptr<Texture>          surface_texture;
    SwapchainImageResource<ImageData> swapchain_resources;
    application::window::Window*      window_container = nullptr;
};
} // namespace gfx::vulkan
