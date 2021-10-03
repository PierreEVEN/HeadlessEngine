#pragma once

#include <vulkan/vulkan.hpp>

class SwapchainConfig
{
  public:
    SwapchainConfig(VkSurfaceKHR surface);

    [[nodiscard]] uint32_t get_image_count() const
    {
        return swapchain_image_count;
    }
    [[nodiscard]] VkSurfaceFormatKHR get_surface_format() const;
    [[nodiscard]] VkPresentModeKHR get_present_mode() const
    {
        return swapchain_present_mode;
    }

    [[nodiscard]] VkSurfaceCapabilitiesKHR get_surface_capabilities() const
    {
        return capabilities;
    }

    [[nodiscard]] bool is_valid() const
    {
        return b_is_valid;
    }

  protected:
    virtual VkSurfaceFormatKHR choose_swapchain_format(VkSurfaceKHR surface, const std::vector<VkSurfaceFormatKHR>& available_formats);
    virtual VkPresentModeKHR   choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkSurfaceCapabilitiesKHR capabilities             = {};
    VkSurfaceFormatKHR       swapchain_surface_format = {.format = VK_FORMAT_UNDEFINED};
    VkPresentModeKHR         swapchain_present_mode   = VK_PRESENT_MODE_IMMEDIATE_KHR;
    uint32_t                 swapchain_image_count    = 0;
    bool                     b_is_valid               = false;
};