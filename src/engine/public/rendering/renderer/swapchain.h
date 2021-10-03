#pragma once

#include "rendering/vulkan/utils.h"

#include <cpputils/eventmanager.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

DECLARE_DELEGATE_MULTICAST(EventOnSwapchainRecreate)

class Surface;
class AMaterial;
class NCamera;

struct SwapchainStatus
{
    bool            is_valid           = false;
    VkCommandBuffer command_buffer     = VK_NULL_HANDLE;
    VkFramebuffer   framebuffer        = VK_NULL_HANDLE;
    uint32_t        image_index        = 0;
    uint32_t        res_x              = 0;
    uint32_t        res_y              = 0;
    AMaterial*      last_used_material = nullptr;
    NCamera*        view               = nullptr;
};

class Swapchain
{
  public:
    Swapchain(Surface* in_surface_target);
    virtual ~Swapchain();

    void            resize_swapchain(const VkExtent2D& new_extend);
    SwapchainStatus prepare_frame();
    void            submit_frame(const SwapchainStatus& context);

    [[nodiscard]] VkSwapchainKHR get_swapchain_khr() const
    {
        return swapchain;
    }

    [[nodiscard]] VkExtent2D get_swapchain_extend() const
    {
        return swapchain_extend;
    }

    [[nodiscard]] uint32_t get_image_count() const
    {
        return swapchain_image_count;
    }
    [[nodiscard]] vulkan_utils::SwapchainSupportDetails get_support_details() const
    {
        return swapchain_support_details;
    }
    [[nodiscard]] VkSurfaceFormatKHR get_surface_format() const
    {
        return swapchain_surface_format;
    }
    [[nodiscard]] VkPresentModeKHR get_present_mode() const
    {
        return swapchain_present_mode;
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
    Surface*                     surface_target             = nullptr;
    bool                         is_swapchain_dirty         = false;
    VkExtent2D                   swapchain_extend           = {};
    VkSwapchainKHR               swapchain                  = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers            = {};
    std::vector<VkSemaphore>     image_acquire_semaphore    = {};
    std::vector<VkSemaphore>     render_finished_semaphores = {};
    std::vector<VkFence>         in_flight_fences           = {};
    std::vector<VkFence>         images_in_flight           = {};

    vulkan_utils::SwapchainSupportDetails swapchain_support_details = {};
    VkSurfaceFormatKHR                    swapchain_surface_format  = {};
    VkPresentModeKHR                      swapchain_present_mode    = VK_PRESENT_MODE_IMMEDIATE_KHR;
    uint32_t                              swapchain_image_count     = 0;
};
