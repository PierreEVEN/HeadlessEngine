

#include "vk_resource_surface.h"

#include "application/application.h"
#include "application/window.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_instance.h"
#include "vulkan/vk_physical_device.h"

#include <vulkan/vulkan_win32.h>

namespace gfx::vulkan
{
SwapchainResource_VK::SwapchainResource_VK(const std::string& name, const CI_Swapchain& create_infos) : surface(create_infos.surface)
{
    /**
     * Get device capabilities
     */
    VkSurfaceCapabilitiesKHR      capabilities;
    std::vector<VkPresentModeKHR> present_modes;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GET_VK_PHYSICAL_DEVICE(), create_infos.surface->surface, &capabilities);

    composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    for (const auto& compositeAlphaFlag : {
             VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
             VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
         })
    {
        if (capabilities.supportedCompositeAlpha & compositeAlphaFlag)
            composite_alpha = compositeAlphaFlag;
    }

    transform_flags = capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : capabilities.currentTransform;

    /**
     * Choose present mode
     */

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(GET_VK_PHYSICAL_DEVICE(), create_infos.surface->surface, &present_mode_count, nullptr);
    if (present_mode_count != 0)
    {
        present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(GET_VK_PHYSICAL_DEVICE(), create_infos.surface->surface, &present_mode_count, present_modes.data());
    }
    present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& mode : present_modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = mode;
        }
    }

    const VkSwapchainCreateInfoKHR swapchain_create_info{
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext                 = nullptr,
        .flags                 = 0,
        .surface               = create_infos.surface->surface,
        .minImageCount         = Device::get().get_frame_count(),
        .imageFormat           = create_infos.surface->surface_format.format,
        .imageColorSpace       = create_infos.surface->surface_format.colorSpace,
        .imageExtent           = VkExtent2D{create_infos.surface->width(), create_infos.surface->height()},
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .preTransform          = transform_flags,
        .compositeAlpha        = composite_alpha,
        .presentMode           = present_mode,
        .clipped               = VK_TRUE,
        .oldSwapchain          = create_infos.previous_swapchain ? create_infos.previous_swapchain->swapchain : VK_NULL_HANDLE,
    };
    VK_CHECK(vkCreateSwapchainKHR(get_device(), &swapchain_create_info, get_allocator(), &swapchain), "Failed to create swap chain");
    uint32_t swapchain_image_count = Device::get().get_frame_count();
    vkGetSwapchainImagesKHR(get_device(), swapchain, &swapchain_image_count, swapchain_images.data());

    for (const auto& image : swapchain_images)
        debug_set_object_name("swapchain images", image);

    debug_set_object_name(name, swapchain);
}

SwapchainResource_VK::~SwapchainResource_VK()
{
    vkDestroySwapchainKHR(get_device(), swapchain, get_allocator());
}

SurfaceResource_VK::SurfaceResource_VK(const std::string& name, const CI_Surface& create_infos) : container(create_infos.container)
{
#if APP_USE_WIN32
    const VkWin32SurfaceCreateInfoKHR surface_create_infos{
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = nullptr,
        .flags     = NULL,
        .hinstance = reinterpret_cast<HINSTANCE>(application::get()->get_platform_app_handle()),
        .hwnd      = reinterpret_cast<HWND>(create_infos.container->get_platform_window_handle()),
    };
    vkCreateWin32SurfaceKHR(get_instance(), &surface_create_infos, get_allocator(), &surface);
    debug_set_object_name(name, surface);
#endif

    /**
     * Choose surface format
     */
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(GET_VK_PHYSICAL_DEVICE(), surface, &format_count, NULL);
    assert(format_count > 0);
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(GET_VK_PHYSICAL_DEVICE(), surface, &format_count, surface_formats.data());

    surface_format.format = VK_FORMAT_UNDEFINED;
    if (format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        surface_format.format     = VK_FORMAT_B8G8R8A8_UNORM;
        surface_format.colorSpace = surface_formats[0].colorSpace;
    }
    else
    {
        bool found_B8G8R8A8_UNORM = false;
        for (auto&& format : surface_formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                surface_format.format     = format.format;
                surface_format.colorSpace = format.colorSpace;
                found_B8G8R8A8_UNORM      = true;
                break;
            }
        }

        if (!found_B8G8R8A8_UNORM)
        {
            surface_format.format     = surface_formats[0].format;
            surface_format.colorSpace = surface_formats[0].colorSpace;
        }
    }

    present_queue = TGpuHandle<QueueResource_VK>("test queue");

    /**
     * Retrieve present queue
     */
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(GET_VK_PHYSICAL_DEVICE(), &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(GET_VK_PHYSICAL_DEVICE(), &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for ([[maybe_unused]] const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(GET_VK_PHYSICAL_DEVICE(), i, surface, &presentSupport), "failed to buffer physical device present support");
        if (presentSupport)
        {
            present_queue_family = i;
            vkGetDeviceQueue(get_device(), present_queue_family, 0, &present_queue->queue);
            break;
        }
        ++i;
    }
}

SurfaceResource_VK::~SurfaceResource_VK()
{
    vkDestroySurfaceKHR(get_instance(), surface, get_allocator());
}

}
