

#include "rendering/swapchain_config.h"

#include <cpputils/logger.hpp>
#include "rendering/graphics.h"
#include "rendering/vulkan/utils.h"

SwapchainConfig::SwapchainConfig(VkSurfaceKHR surface_khr)
{
    const VkPhysicalDevice physical_device = Graphics::get()->get_physical_device();

    const auto swapchain_support_details = vulkan_utils::get_swapchain_support_details(surface_khr, physical_device);

    if (swapchain_support_details.formats.empty() || swapchain_support_details.present_modes.empty())
        return;

    b_is_valid               = true;
    capabilities             = swapchain_support_details.capabilities;
    swapchain_surface_format = choose_swapchain_format(surface_khr, swapchain_support_details.formats);
    swapchain_present_mode   = choose_swapchain_present_mode(swapchain_support_details.present_modes);
    swapchain_image_count    = capabilities.minImageCount + 1;
}

VkSurfaceFormatKHR SwapchainConfig::get_surface_format() const
{
    if (swapchain_surface_format.format == VK_FORMAT_UNDEFINED)
        LOG_FATAL("swapchain format is undefined");
    return swapchain_surface_format;
}

VkSurfaceFormatKHR SwapchainConfig::choose_swapchain_format(VkSurfaceKHR surface, const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    const VkPhysicalDevice device = Graphics::get()->get_physical_device();
    uint32_t               formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
    assert(formatCount > 0);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, surfaceFormats.data());

    VkSurfaceFormatKHR format{.format = VK_FORMAT_UNDEFINED};

    // If the surface log_format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferered log_format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        format.format     = VK_FORMAT_B8G8R8A8_UNORM;
        format.colorSpace = surfaceFormats[0].colorSpace;
    }
    else
    {
        // iterate over the list of available surface log_format and
        // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
        bool found_B8G8R8A8_UNORM = false;
        for (auto&& surfaceFormat : surfaceFormats)
        {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                format.format        = surfaceFormat.format;
                format.colorSpace    = surfaceFormat.colorSpace;
                found_B8G8R8A8_UNORM = true;
                break;
            }
        }

        // in case VK_FORMAT_B8G8R8A8_UNORM is not available
        // select the first available color log_format
        if (!found_B8G8R8A8_UNORM)
        {
            format.format     = surfaceFormats[0].format;
            format.colorSpace = surfaceFormats[0].colorSpace;
        }
    }
    return format;
}

VkPresentModeKHR SwapchainConfig::choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}