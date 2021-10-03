

#include "rendering/renderer/swapchain.h"

#include "config.h"
#include "rendering/renderer/surface.h"
#include "rendering/gfx_context.h"
#include "rendering/vulkan/command_pool.h"
#include "rendering/vulkan/common.h"
#include <cpputils/logger.hpp>

Swapchain::Swapchain(Surface* in_surface_target) : surface_target(in_surface_target)
{
    swapchain_support_details = vulkan_utils::get_swapchain_support_details(in_surface_target->get_surface(), GfxContext::get()->physical_device);
    swapchain_surface_format  = vulkan_utils::choose_swapchain_surface_format(in_surface_target->get_surface());
    swapchain_present_mode    = vulkan_utils::choose_swapchain_present_mode(swapchain_support_details.present_modes);
    swapchain_image_count     = swapchain_support_details.capabilities.minImageCount + 1;

    int width = 0, height = 0;
    glfwGetFramebufferSize(surface_target->get_handle(), &width, &height);
    swapchain_extend = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    recreate_frame_objects();
    check_resize_swapchain(true);
        
    LOG_INFO("swapchain details : \n\
		\t-image count : %d\n\
		\t-present mode : %d\n\
		\t-surface log_format : %d\
		",
             swapchain_image_count, swapchain_present_mode, swapchain_surface_format);
}

Swapchain::~Swapchain()
{
    destroy_frame_objects();
    destroy_swapchain();
}

void Swapchain::resize_swapchain(const VkExtent2D& new_extend)
{
    if (swapchain_extend.width == new_extend.width && swapchain_extend.height == new_extend.height)
        return;

    swapchain_extend = new_extend;
    recreate_swapchain();
}

SwapchainStatus Swapchain::prepare_frame()
{
    vkWaitForFences(GfxContext::get()->logical_device, 1, &in_flight_fences[current_frame_id], VK_TRUE, UINT64_MAX);

    // Retrieve the next available image ID
    uint32_t       image_index;
    const VkResult result = vkAcquireNextImageKHR(GfxContext::get()->logical_device, swapchain, UINT64_MAX, image_acquire_semaphore[current_frame_id], VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swapchain();
        return {};
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOG_ERROR("Failed to acquire image from the swapchain");
        return {};
    }

    // Ensure the selected image is available.
    if (images_in_flight[image_index] != VK_NULL_HANDLE)
        vkWaitForFences(GfxContext::get()->logical_device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    images_in_flight[image_index] = in_flight_fences[current_frame_id];

    const SwapchainStatus render_context{
        .is_valid       = true,
        .command_buffer = command_buffers[image_index],
        .framebuffer    = nullptr,
        .image_index    = image_index,
        .res_x          = swapchain_extend.width,
        .res_y          = swapchain_extend.height,
    };

    /**
     * Prepare command buffer
     */

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags            = 0;       // Optional
    begin_info.pInheritanceInfo = nullptr; // Optional

    VK_ENSURE(vkBeginCommandBuffer(render_context.command_buffer, &begin_info), stringutils::format("Failed to create command buffer #%d", image_index).c_str());

    return render_context;
}

void Swapchain::submit_frame(const SwapchainStatus& context)
{
    /**
     * Submit queues
     */

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore          acquire_wait_semaphore[] = {image_acquire_semaphore[current_frame_id]};
    VkPipelineStageFlags wait_stage[]             = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount                 = 1;
    submitInfo.pWaitSemaphores                    = acquire_wait_semaphore;
    submitInfo.pWaitDstStageMask                  = wait_stage;
    submitInfo.commandBufferCount                 = 1;
    submitInfo.pCommandBuffers                    = &context.command_buffer;

    VkSemaphore finished_semaphore[] = {render_finished_semaphores[current_frame_id]}; // This fence is used to tell when the gpu can present the submitted data
    submitInfo.signalSemaphoreCount  = 1;
    submitInfo.pSignalSemaphores     = finished_semaphore;

    /** Submit command buffers */
    vkResetFences(GfxContext::get()->logical_device, 1, &in_flight_fences[current_frame_id]);
    GfxContext::get()->submit_graphic_queue(submitInfo, in_flight_fences[current_frame_id]); // Pass fence to know when all the data are submitted

    /**
     * Present to swapchain
     */
    VkSwapchainKHR         swapChains[] = {swapchain};
    const VkPresentInfoKHR present_infos{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = finished_semaphore,
        .swapchainCount     = 1,
        .pSwapchains        = swapChains,
        .pImageIndices      = &context.image_index,
        .pResults           = nullptr,
    };

    const VkResult submit_result = GfxContext::get()->submit_present_queue(present_infos);

    if (submit_result == VK_ERROR_OUT_OF_DATE_KHR || submit_result == VK_SUBOPTIMAL_KHR || is_swapchain_dirty)
    {
        recreate_swapchain();
    }
    else if (submit_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to present image to swap chain");
    }

    // Select next image
    current_frame_id = (current_frame_id + 1) % config::max_frame_in_flight;
}

void Swapchain::recreate_frame_objects()
{
    destroy_frame_objects();

    command_buffers.resize(get_image_count(), VK_NULL_HANDLE);
    image_acquire_semaphore.resize(config::max_frame_in_flight, VK_NULL_HANDLE);
    render_finished_semaphores.resize(config::max_frame_in_flight, VK_NULL_HANDLE);
    in_flight_fences.resize(config::max_frame_in_flight, VK_NULL_HANDLE);
    images_in_flight.resize(get_image_count(), VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo command_buffer_infos{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = command_pool::get(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
    };

    VkSemaphoreCreateInfo semaphore_infos{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    VkFenceCreateInfo fence_infos{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    VK_ENSURE(vkAllocateCommandBuffers(GfxContext::get()->logical_device, &command_buffer_infos, command_buffers.data()), "Failed to allocate command buffer");
    for (size_t i = 0; i < config::max_frame_in_flight; i++)
    {
        VK_ENSURE(vkCreateSemaphore(GfxContext::get()->logical_device, &semaphore_infos, vulkan_common::allocation_callback, &image_acquire_semaphore[i]), "Failed to create image available semaphore #%d" + i);
        VK_ENSURE(vkCreateSemaphore(GfxContext::get()->logical_device, &semaphore_infos, vulkan_common::allocation_callback, &render_finished_semaphores[i]), "Failed to create render finnished semaphore #%d" + i)
        VK_ENSURE(vkCreateFence(GfxContext::get()->logical_device, &fence_infos, vulkan_common::allocation_callback, &in_flight_fences[i]), "Failed to create fence #%d" + i);
    }
}

void Swapchain::destroy_frame_objects()
{
    if (!command_buffers.empty())
        vkFreeCommandBuffers(GfxContext::get()->logical_device, command_pool::get(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());

    for (const auto& semaphore : render_finished_semaphores)
        vkDestroySemaphore(GfxContext::get()->logical_device, semaphore, vulkan_common::allocation_callback);

    for (const auto& semaphore : image_acquire_semaphore)
        vkDestroySemaphore(GfxContext::get()->logical_device, semaphore, vulkan_common::allocation_callback);

    for (const auto& fence : in_flight_fences)
        vkDestroyFence(GfxContext::get()->logical_device, fence, vulkan_common::allocation_callback);

    command_buffers.clear();
    render_finished_semaphores.clear();
    image_acquire_semaphore.clear();
    in_flight_fences.clear();
}

void Swapchain::check_resize_swapchain(bool b_force)
{
    if (is_swapchain_dirty || b_force)
        recreate_swapchain();

    is_swapchain_dirty = false;
}

void Swapchain::recreate_swapchain()
{
    if (swapchain_extend.width == 0 || swapchain_extend.height == 0)
        return;

    if (swapchain != VK_NULL_HANDLE)
        destroy_swapchain();

    swapchain_extend = swapchain_extend;

    if (swapchain_extend.width == 0 || swapchain_extend.height == 0)
        LOG_FATAL("cannot recreate swapchain with zero extend.");

    VkSwapchainCreateInfoKHR create_info{
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface               = surface_target->get_surface(),
        .minImageCount         = get_image_count(),
        .imageFormat           = get_surface_format().format,
        .imageColorSpace       = get_surface_format().colorSpace,
        .imageExtent           = swapchain_extend,
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .preTransform          = get_surface_transformation_flags(),
        .compositeAlpha        = select_composite_alpha_flags(),
        .presentMode           = get_present_mode(),
        .clipped               = VK_TRUE,
        .oldSwapchain          = VK_NULL_HANDLE,
    };

    if (GfxContext::get()->queue_families.graphic_family != GfxContext::get()->queue_families.present_family)
    {
        std::vector queue_family_indices  = {GfxContext::get()->queue_families.graphic_family.value(), GfxContext::get()->queue_families.present_family.value()};
        create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
        create_info.pQueueFamilyIndices   = queue_family_indices.data();
    }

    LOG_INFO("swapchain size : (%d x %d)", swapchain_extend.width, swapchain_extend.height);

    VK_ENSURE(vkCreateSwapchainKHR(GfxContext::get()->logical_device, &create_info, vulkan_common::allocation_callback, &swapchain), "Failed to create swap chain");
    VK_CHECK(swapchain, stringutils::format("Invalid swapchain reference : ( %d x %d ) / sharing mode : %d", swapchain_extend.width, swapchain_extend.height, create_info.imageSharingMode).c_str());

    on_swapchain_recreate.execute();
}

void Swapchain::destroy_swapchain()
{
    vkDestroySwapchainKHR(GfxContext::get()->logical_device, swapchain, vulkan_common::allocation_callback);
    swapchain = VK_NULL_HANDLE;
}

VkCompositeAlphaFlagBitsKHR Swapchain::select_composite_alpha_flags() const
{
    for (auto& compositeAlphaFlag : {
             VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
             VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
         })
    {
        if (get_support_details().capabilities.supportedCompositeAlpha & compositeAlphaFlag)
            return compositeAlphaFlag;
    }
    return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

VkSurfaceTransformFlagBitsKHR Swapchain::get_surface_transformation_flags() const
{
    // Find the transformation of the surface
    if (get_support_details().capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        // We prefer a non-rotated transform
        return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        return get_support_details().capabilities.currentTransform;
}
