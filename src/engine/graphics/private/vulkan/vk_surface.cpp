
#include "vk_surface.h"

#include "application/application.h"
#include "gfx/physical_device.h"
#include "vk_helper.h"
#include "vk_render_pass.h"
#include "vk_render_pass_instance.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_command_buffer.h"
#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_instance.h"
#include "vulkan/vk_physical_device.h"
#include "vulkan/vk_texture.h"

#include "vulkan/vk_errors.h"
#include "vulkan/vk_unit.h"

namespace gfx::vulkan
{
Surface_VK::Surface_VK(application::window::Window* container) : window_container(container)
{
    /**
     * Create surface
     */
#if APP_USE_WIN32
    VkWin32SurfaceCreateInfoKHR create_infos = {
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = nullptr,
        .flags     = NULL,
        .hinstance = reinterpret_cast<HINSTANCE>(application::get()->get_platform_app_handle()),
        .hwnd      = reinterpret_cast<HWND>(container->get_platform_window_handle()),
    };

    vkCreateWin32SurfaceKHR(get_instance(), &create_infos, get_allocator(), &surface);
    debug_set_object_name(stringutils::format("surface %s (win32)", window_container->name().c_str()), surface);
#endif

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
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(GET_VK_PHYSICAL_DEVICE(), i, surface, &presentSupport), "failed to get physical device present support");
        if (presentSupport)
        {
            present_queue_family = i;
            vkGetDeviceQueue(get_device(), present_queue_family, 0, &present_queue);
            break;
        }
        ++i;
    }

    /**
     * Get device capabilities
     */
    VkSurfaceCapabilitiesKHR      capabilities;
    std::vector<VkPresentModeKHR> present_modes;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GET_VK_PHYSICAL_DEVICE(), surface, &capabilities);

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

    /**
     * Choose present mode
     */

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(GET_VK_PHYSICAL_DEVICE(), surface, &present_mode_count, nullptr);
    if (present_mode_count != 0)
    {
        present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(GET_VK_PHYSICAL_DEVICE(), surface, &present_mode_count, present_modes.data());
    }
    present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& mode : present_modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = mode;
        }
    }

    const VkSemaphoreCreateInfo semaphore_infos{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    const VkFenceCreateInfo fence_infos{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    uint8_t resource_index = 0;
    for (auto& resource : swapchain_resources)
    {
        VK_CHECK(vkCreateSemaphore(get_device(), &semaphore_infos, get_allocator(), &resource.image_acquire_semaphore), "Failed to create images acquire semaphore");
        VK_CHECK(vkCreateFence(get_device(), &fence_infos, get_allocator(), &resource.in_flight_fence), "Failed to create fence");
        debug_set_object_name(stringutils::format("surface %s : semaphore image acquire #%d", window_container->name().c_str(), resource_index), resource.image_acquire_semaphore);
        debug_set_object_name(stringutils::format("surface %s : fence image in flight #%d", window_container->name().c_str(), resource_index), resource.in_flight_fence);

        resource.image_in_flight = VK_NULL_HANDLE;
        ++resource_index;
    }

    recreate_swapchain();
}

Surface_VK::~Surface_VK()
{
    vkDeviceWaitIdle(get_device());
    vkDestroySwapchainKHR(get_device(), swapchain, get_allocator());
    vkDestroySurfaceKHR(get_instance(), surface, get_allocator());
    for (const auto& elem : swapchain_resources)
    {
        vkDestroySemaphore(get_device(), elem.image_acquire_semaphore, get_allocator());
        vkDestroyFence(get_device(), elem.in_flight_fence, get_allocator());
    }
}

void Surface_VK::render()
{
    if (swapchain_resources->image_in_flight)
        VK_CHECK(vkWaitForFences(get_device(), 1, &swapchain_resources->image_in_flight, VK_TRUE, UINT64_MAX), "wait failed");

    // Don't draw_pass if window is minimized
    if (window_container->width() == 0 || window_container->height() == 0)
        return;

    const VkSemaphore& image_acquire_semaphore = swapchain_resources->image_acquire_semaphore;

    // Retrieve the next available images ID
    uint32_t       image_index;
    const VkResult result = vkAcquireNextImageKHR(get_device(), swapchain, UINT64_MAX, image_acquire_semaphore, VK_NULL_HANDLE, &image_index);
    set_frame(static_cast<uint8_t>(image_index));

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        LOG_FATAL("framebuffer resizing is not implemented yet");
        // recreate_swapchain();
        // return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOG_ERROR("Failed to acquire images from the swapchain");
        return;
    }

    // The present pass must wait for the image acquire semaphore
    dynamic_cast<RenderPassInstance_VK*>(main_render_pass.get())->swapchain_image_acquire_semaphore = image_acquire_semaphore;

    // Draw content
    main_render_pass->draw_pass();

    // Submit to present queue
    const VkPresentInfoKHR present_infos{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &*dynamic_cast<RenderPassInstance_VK*>(main_render_pass.get())->render_finished_semaphore,
        .swapchainCount     = 1,
        .pSwapchains        = &swapchain,
        .pImageIndices      = &image_index,
        .pResults           = nullptr,
    };
    const VkResult submit_result = vkQueuePresentKHR(present_queue, &present_infos);

    if (submit_result == VK_ERROR_OUT_OF_DATE_KHR || submit_result == VK_SUBOPTIMAL_KHR)
    {
        LOG_FATAL("framebuffer resizing is not implemented yet");
        // recreate_swapchain();
    }
    else if (submit_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to present images to swap chain");
    }
}

void Surface_VK::recreate_swapchain()
{
    if (window_container->width() == 0 || window_container->height() == 0)
        return;

    if (swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(get_device(), swapchain, get_allocator());

    const VkSwapchainCreateInfoKHR create_info{
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext                 = nullptr,
        .flags                 = 0,
        .surface               = surface,
        .minImageCount         = get_image_count(),
        .imageFormat           = surface_format.format,
        .imageColorSpace       = surface_format.colorSpace,
        .imageExtent           = VkExtent2D{window_container->width(), window_container->height()},
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .preTransform          = transform_flags,
        .compositeAlpha        = composite_alpha,
        .presentMode           = present_mode,
        .clipped               = VK_TRUE,
        .oldSwapchain          = VK_NULL_HANDLE,
    };

    VK_CHECK(vkCreateSwapchainKHR(get_device(), &create_info, get_allocator(), &swapchain), "Failed to create swap chain");

    debug_set_object_name(stringutils::format("surface %s : swapchain", window_container->name().c_str()), swapchain);

    uint32_t             swapchain_image_count = get_image_count();
    std::vector<VkImage> swapchain_images(get_image_count(), VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(get_device(), swapchain, &swapchain_image_count, swapchain_images.data());

    SwapchainImageResource<VkImage> images{};
    for (uint8_t i = 0; i < images.get_max_instance_count(); ++i)
    {
        images[i] = swapchain_images[i];
        debug_set_object_name(stringutils::format("surface %s : swapchain image #%d", window_container->name().c_str(), i), images[i]);
    }
    surface_texture = std::make_shared<Texture_VK>(window_container->width(), window_container->height(), 1,
                                                   TextureParameter{
                                                       .format                 = Texture_VK::engine_texture_format_from_vk(get_surface_format().format),
                                                       .image_type             = EImageType::Texture_2D,
                                                       .transfer_capabilities  = ETextureTransferCapabilities::None,
                                                       .gpu_write_capabilities = ETextureGPUWriteCapabilities::Enabled,
                                                       .gpu_read_capabilities  = ETextureGPUReadCapabilities::None,
                                                       .mip_level              = 1,
                                                       .read_only              = false,
                                                   },
                                                   images);
}

} // namespace gfx::vulkan