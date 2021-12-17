#pragma once

#include "render_pass_description.h"
#include "rendering/vulkan/utils.h"
#include "swapchain_image_resource.h"

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

    void           resize_swapchain(const VkExtent2D& new_extend);
    SwapchainFrame acquire_frame();
    void           submit_frame(const SwapchainFrame& context);

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

    uint32_t       current_frame_id   = 0;
    GfxInterface*  graphic_instance   = nullptr;
    bool           is_swapchain_dirty = false;
    VkExtent2D     swapchain_extend   = {};
    VkSwapchainKHR swapchain_khr      = VK_NULL_HANDLE;

    struct ImageData
    {
        VkCommandBuffer command_buffer   = VK_NULL_HANDLE;
        VkFence         images_in_flight = VK_NULL_HANDLE;
    };
    struct InFlightData
    {
        VkSemaphore image_acquire_semaphore    = VK_NULL_HANDLE;
        VkSemaphore render_finished_semaphores = VK_NULL_HANDLE;
        VkFence     in_flight_fences           = VK_NULL_HANDLE;
    };

    SwapchainImageResource<ImageData>    per_image_data = {};
    SwapchainImageResource<InFlightData> in_flight_data = {};
};
