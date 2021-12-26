
#include "vk_surface.h"

#include "allocator.h"
#include "application/application.h"
#include "command_pool.h"
#include "device.h"
#include "gfx/physical_device.h"
#include "instance.h"
#include "vk_physical_device.h"

#include "vulkan/assertion.h"
#include "vulkan/unit.h"

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

    const VkCommandBufferAllocateInfo command_buffer_infos{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = command_pool::get(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    const VkSemaphoreCreateInfo semaphore_infos{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    const VkFenceCreateInfo fence_infos{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (auto& resource : swapchain_resources)
    {
        LOG_WARNING("create 0 : ");
        VK_CHECK(vkAllocateCommandBuffers(get_device(), &command_buffer_infos, &resource.command_buffer), "Failed to allocate command buffer");
        VK_CHECK(vkCreateSemaphore(get_device(), &semaphore_infos, get_allocator(), &resource.image_acquire_semaphore), "Failed to create image acquire semaphore");
        VK_CHECK(vkCreateSemaphore(get_device(), &semaphore_infos, get_allocator(), &resource.render_finnished_semaphore), "Failed to create render finnished semaphore");
        VK_CHECK(vkCreateFence(get_device(), &fence_infos, get_allocator(), &resource.in_flight_fence), "Failed to create fence");
        resource.image_in_flight = VK_NULL_HANDLE;
    }

    recreate_swapchain();
}

Surface_VK::~Surface_VK()
{
    vkDestroySurfaceKHR(get_instance(), surface, get_allocator());
}

void Surface_VK::display(RenderTarget& render_target)
{
    LOG_WARNING("A");
    if (swapchain_resources->image_in_flight)
        VK_CHECK(vkWaitForFences(get_device(), 1, &swapchain_resources->image_in_flight, VK_TRUE, UINT64_MAX), "wait failed");

    LOG_WARNING("1 : %x", window_container);
    // Don't render if window is minimized
    if (window_container->width() == 0 || window_container->height() == 0)
        return;

    LOG_WARNING("2 : %x", swapchain_resources->image_acquire_semaphore);
    // Retrieve the next available image ID
    uint32_t       image_index;
    const VkResult result = vkAcquireNextImageKHR(get_device(), swapchain, UINT64_MAX, swapchain_resources->image_acquire_semaphore, VK_NULL_HANDLE, &image_index);

    LOG_WARNING("a");
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swapchain();
        return;
    }

    LOG_WARNING("3");
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOG_ERROR("Failed to acquire image from the swapchain");
        return;
    }

    LOG_WARNING("B");

    /**
     * Prepare command buffer
     */

    const VkCommandBufferBeginInfo begin_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags            = 0,
        .pInheritanceInfo = nullptr,
    };

    VK_CHECK(vkBeginCommandBuffer(swapchain_resources->command_buffer, &begin_info), "Failed to create command buffer #%d", image_index);
    //àTODO
    /**
     * Submit queues
     */
    VK_CHECK(vkEndCommandBuffer(swapchain_resources->command_buffer), "Failed to register command buffer #d", image_index);

    VkPipelineStageFlags wait_stage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    LOG_WARNING("C");

    const VkSubmitInfo submit_infos{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &swapchain_resources->image_acquire_semaphore,
        .pWaitDstStageMask    = wait_stage,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &swapchain_resources->command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &swapchain_resources->render_finnished_semaphore,
    };

    swapchain_resources->image_in_flight = get_physical_device<VulkanPhysicalDevice>()->submit_queue(EQueueFamilyType::GRAPHIC_QUEUE, submit_infos);

    /**
     * Present to swapchain
     */

    const VkPresentInfoKHR present_infos{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &swapchain_resources->render_finnished_semaphore,
        .swapchainCount     = 1,
        .pSwapchains        = &swapchain,
        .pImageIndices      = &image_index,
        .pResults           = nullptr,
    };

    LOG_WARNING("D");

    const VkResult submit_result = vkQueuePresentKHR(present_queue, &present_infos);

    if (submit_result == VK_ERROR_OUT_OF_DATE_KHR || submit_result == VK_SUBOPTIMAL_KHR)
    {
        recreate_swapchain();
    }
    else if (submit_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to present image to swap chain");
    }
    LOG_WARNING("E");
}

void Surface_VK::recreate_swapchain()
{
    LOG_DEBUG("1");
    if (window_container->width() == 0 || window_container->height() == 0)
        return;

    LOG_DEBUG("2");
    if (swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(get_device(), swapchain, get_allocator());

    VkSwapchainCreateInfoKHR create_info{
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

    const QueueInfo& graphic_queue = get_physical_device<VulkanPhysicalDevice>()->get_queue_family(EQueueFamilyType::GRAPHIC_QUEUE, 0);

    if (graphic_queue.queue_index != present_queue_family && false)
    {
        const std::vector queue_family_indices = {graphic_queue.queue_index, present_queue_family};
        create_info.imageSharingMode           = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount      = static_cast<uint32_t>(queue_family_indices.size());
        create_info.pQueueFamilyIndices        = queue_family_indices.data();
    }
    LOG_DEBUG("3 %d", get_image_count());

    VK_CHECK(vkCreateSwapchainKHR(get_device(), &create_info, get_allocator(), &swapchain), "Failed to create swap chain")
    LOG_DEBUG("4");
}

} // namespace gfx::vulkan