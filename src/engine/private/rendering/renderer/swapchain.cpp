

#include "rendering/renderer/swapchain.h"

#include "config.h"
#include "rendering/graphics.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/command_pool.h"
#include "rendering/vulkan/common.h"
#include <cpputils/logger.hpp>

Swapchain::Swapchain(GfxInterface* in_graphic_instance) : graphic_instance(in_graphic_instance)
{

    int width = 0, height = 0;
    glfwGetFramebufferSize(graphic_instance->get_glfw_handle(), &width, &height);
    swapchain_extend = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    recreate_frame_objects();
    check_resize_swapchain(true);
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

SwapchainFrame Swapchain::acquire_frame()
{
    vkWaitForFences(Graphics::get()->get_logical_device(), 1, &in_flight_fences[current_frame_id], VK_TRUE, UINT64_MAX);

    // Don't render if window is minimized
    if (get_swapchain_extend().width == 0 || get_swapchain_extend().height == 0)
        return {};

    // Retrieve the next available image ID
    uint32_t       image_index;
    const VkResult result = vkAcquireNextImageKHR(Graphics::get()->get_logical_device(), swapchain_khr, UINT64_MAX, image_acquire_semaphore[current_frame_id], VK_NULL_HANDLE, &image_index);
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
        vkWaitForFences(Graphics::get()->get_logical_device(), 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    images_in_flight[image_index] = in_flight_fences[current_frame_id];

    const SwapchainFrame render_context{
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

    const VkCommandBufferBeginInfo begin_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags            = 0,
        .pInheritanceInfo = nullptr,
    };

    VK_ENSURE(vkBeginCommandBuffer(render_context.command_buffer, &begin_info), stringutils::format("Failed to create command buffer #%d", image_index).c_str());

    return render_context;
}

void Swapchain::submit_frame(const SwapchainFrame& context)
{
    /**
     * Submit queues
     */
    VK_ENSURE(vkEndCommandBuffer(context.command_buffer), "Failed to register command buffer #d", context.image_index);

    VkPipelineStageFlags wait_stage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    const VkSubmitInfo submit_infos{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &image_acquire_semaphore[current_frame_id],
        .pWaitDstStageMask    = wait_stage,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &context.command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &render_finished_semaphores[current_frame_id],
    };

    /** Submit command buffers */
    vkResetFences(Graphics::get()->get_logical_device(), 1, &in_flight_fences[current_frame_id]);
    Graphics::get()->submit_graphic_queue(submit_infos, in_flight_fences[current_frame_id]); // Pass fence to know when all the data are submitted

    /**
     * Present to swapchain
     */

    const VkPresentInfoKHR present_infos{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &render_finished_semaphores[current_frame_id],
        .swapchainCount     = 1,
        .pSwapchains        = &swapchain_khr,
        .pImageIndices      = &context.image_index,
        .pResults           = nullptr,
    };

    const VkResult submit_result = Graphics::get()->submit_present_queue(present_infos);

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

    const uint32_t image_count = Graphics::get()->get_swapchain_config()->get_image_count();

    command_buffers.resize(image_count, VK_NULL_HANDLE);
    image_acquire_semaphore.resize(config::max_frame_in_flight, VK_NULL_HANDLE);
    render_finished_semaphores.resize(config::max_frame_in_flight, VK_NULL_HANDLE);
    in_flight_fences.resize(config::max_frame_in_flight, VK_NULL_HANDLE);
    images_in_flight.resize(image_count, VK_NULL_HANDLE);

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

    VK_ENSURE(vkAllocateCommandBuffers(Graphics::get()->get_logical_device(), &command_buffer_infos, command_buffers.data()), "Failed to allocate command buffer");
    for (size_t i = 0; i < config::max_frame_in_flight; i++)
    {
        VK_ENSURE(vkCreateSemaphore(Graphics::get()->get_logical_device(), &semaphore_infos, vulkan_common::allocation_callback, &image_acquire_semaphore[i]), "Failed to create image available semaphore #%d" + i);
        VK_ENSURE(vkCreateSemaphore(Graphics::get()->get_logical_device(), &semaphore_infos, vulkan_common::allocation_callback, &render_finished_semaphores[i]), "Failed to create render finnished semaphore #%d" + i)
        VK_ENSURE(vkCreateFence(Graphics::get()->get_logical_device(), &fence_infos, vulkan_common::allocation_callback, &in_flight_fences[i]), "Failed to create fence #%d" + i);
    }
}

void Swapchain::destroy_frame_objects()
{
    if (!command_buffers.empty())
        vkFreeCommandBuffers(Graphics::get()->get_logical_device(), command_pool::get(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());

    for (const auto& semaphore : render_finished_semaphores)
        vkDestroySemaphore(Graphics::get()->get_logical_device(), semaphore, vulkan_common::allocation_callback);

    for (const auto& semaphore : image_acquire_semaphore)
        vkDestroySemaphore(Graphics::get()->get_logical_device(), semaphore, vulkan_common::allocation_callback);

    for (const auto& fence : in_flight_fences)
        vkDestroyFence(Graphics::get()->get_logical_device(), fence, vulkan_common::allocation_callback);

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

    if (swapchain_khr != VK_NULL_HANDLE)
        destroy_swapchain();

    swapchain_extend = swapchain_extend;

    if (swapchain_extend.width == 0 || swapchain_extend.height == 0)
        LOG_FATAL("cannot recreate swapchain with zero extend.");

    auto* swapchain_settings = Graphics::get()->get_swapchain_config();

    VkSwapchainCreateInfoKHR create_info{
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface               = graphic_instance->get_surface_khr(),
        .minImageCount         = swapchain_settings->get_image_count(),
        .imageFormat           = swapchain_settings->get_surface_format().format,
        .imageColorSpace       = swapchain_settings->get_surface_format().colorSpace,
        .imageExtent           = swapchain_extend,
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .preTransform          = get_surface_transformation_flags(),
        .compositeAlpha        = select_composite_alpha_flags(),
        .presentMode           = swapchain_settings->get_present_mode(),
        .clipped               = VK_TRUE,
        .oldSwapchain          = VK_NULL_HANDLE,
    };

    if (Graphics::get()->get_queue_family_indices().graphic_family != Graphics::get()->get_queue_family_indices().present_family)
    {
        std::vector queue_family_indices  = {Graphics::get()->get_queue_family_indices().graphic_family.value(), Graphics::get()->get_queue_family_indices().present_family.value()};
        create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
        create_info.pQueueFamilyIndices   = queue_family_indices.data();
    }

    VK_ENSURE(vkCreateSwapchainKHR(Graphics::get()->get_logical_device(), &create_info, vulkan_common::allocation_callback, &swapchain_khr), "Failed to create swap chain");
    VK_CHECK(swapchain_khr, stringutils::format("Invalid swapchain reference : ( %d x %d ) / sharing mode : %d", swapchain_extend.width, swapchain_extend.height, create_info.imageSharingMode).c_str());

    on_swapchain_recreate.execute();
}

void Swapchain::destroy_swapchain()
{
    vkDestroySwapchainKHR(Graphics::get()->get_logical_device(), swapchain_khr, vulkan_common::allocation_callback);
    swapchain_khr = VK_NULL_HANDLE;
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
        if (Graphics::get()->get_swapchain_config()->get_surface_capabilities().supportedCompositeAlpha & compositeAlphaFlag)
            return compositeAlphaFlag;
    }
    return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

VkSurfaceTransformFlagBitsKHR Swapchain::get_surface_transformation_flags() const
{
    // Find the transformation of the surface
    if (Graphics::get()->get_swapchain_config()->get_surface_capabilities().supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        // We prefer a non-rotated transform
        return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        return Graphics::get()->get_swapchain_config()->get_surface_capabilities().currentTransform;
}
