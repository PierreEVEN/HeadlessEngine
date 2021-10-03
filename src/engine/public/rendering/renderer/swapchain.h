#pragma once

#include "render_pass_description.h"
#include "rendering/vulkan/utils.h"

#include <cpputils/eventmanager.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

DECLARE_DELEGATE_MULTICAST(EventOnSwapchainRecreate)

class GfxInterface;
class NCamera;

class Swapchain
{
  public:
    Swapchain(GfxInterface* in_graphic_instance);
    virtual ~Swapchain();

    void            resize_swapchain(const VkExtent2D& new_extend);
    SwapchainFrame acquire_frame();
    void            submit_frame(const SwapchainFrame& context);

    [[nodiscard]] VkSwapchainKHR get_swapchain_khr() const
    {
        return swapchain_khr;
    }

    [[nodiscard]] VkExtent2D get_swapchain_extend() const
    {
        return swapchain_extend;
    }

    EventOnSwapchainRecreate on_swapchain_recreate;

  private:
    void recreate_frame_objects();
    void destroy_frame_objects();

    void check_resize_swapchain(bool b_force = false);
    void recreate_swapchain();
    void destroy_swapchain();

    [[nodiscard]] VkCompositeAlphaFlagBitsKHR   select_composite_alpha_flags() const;
    [[nodiscard]] VkSurfaceTransformFlagBitsKHR get_surface_transformation_flags() const;

    uint32_t                     current_frame_id           = 0;
    GfxInterface*                 graphic_instance           = nullptr;
    bool                         is_swapchain_dirty         = false;
    VkExtent2D                   swapchain_extend           = {};
    VkSwapchainKHR               swapchain_khr                  = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers            = {};
    std::vector<VkSemaphore>     image_acquire_semaphore    = {};
    std::vector<VkSemaphore>     render_finished_semaphores = {};
    std::vector<VkFence>         in_flight_fences           = {};
    std::vector<VkFence>         images_in_flight           = {};
};
