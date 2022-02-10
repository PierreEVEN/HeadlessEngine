
#include "vk_surface.h"

#include "application/application.h"
#include "gfx/physical_device.h"
#include "vk_helper.h"
#include "vk_render_pass.h"
#include "vk_render_pass_instance.h"
#include "vk_types.h"
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
SwapchainResource_VK::SwapchainResource_VK(const std::string& name, const CI_Swapchain& create_infos) : parameters(create_infos)
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
        .imageExtent           = VkExtent2D{create_infos.surface->parameters.container->absolute_width(), create_infos.surface->parameters.container->absolute_height()},
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

SurfaceResource_VK::SurfaceResource_VK(const std::string& name, const CI_Surface& create_infos) : parameters(create_infos)
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

Surface_VK::Surface_VK(application::window::Window* container) : window_container(container)
{
    surface = TGpuHandle<SurfaceResource_VK>("surface test", SurfaceResource_VK::CI_Surface{
                                                                 .container = window_container,
                                                             });

    for (auto& resource : swapchain_resources)
        resource.image_acquire_semaphore = TGpuHandle<SemaphoreResource_VK>("semaphore", SemaphoreResource_VK::CI_Semaphore{});

    recreate_swapchain();
}

Surface_VK::~Surface_VK()
{
}

void Surface_VK::render()
{
    // Don't draw_pass if window is minimized
    if (window_container->absolute_width() == 0 || window_container->absolute_height() == 0)
        return;

    const auto& image_acquire_semaphore = swapchain_resources->image_acquire_semaphore;

    // Retrieve the next available images ID
    uint32_t       image_index;
    const VkResult result = vkAcquireNextImageKHR(get_device(), swapchain->swapchain, UINT64_MAX, image_acquire_semaphore->semaphore, VK_NULL_HANDLE, &image_index);
    LOG_WARNING("begin image %d", image_index);

    if (auto& render_finished_fence = dynamic_cast<RenderPassInstance_VK*>(main_render_pass.get())->frame_data->render_finished_fences)
        render_finished_fence->wait_fence();

    Device::get().begin_frame(static_cast<uint8_t>(image_index));
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swapchain();
        return;
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
        .pWaitSemaphores    = &dynamic_cast<RenderPassInstance_VK*>(main_render_pass.get())->frame_data->render_finished_semaphore->semaphore,
        .swapchainCount     = 1,
        .pSwapchains        = &swapchain->swapchain,
        .pImageIndices      = &image_index,
        .pResults           = nullptr,
    };
    const VkResult submit_result = vkQueuePresentKHR(surface->present_queue->queue, &present_infos);

    if (submit_result == VK_ERROR_OUT_OF_DATE_KHR || submit_result == VK_SUBOPTIMAL_KHR)
    {
        recreate_swapchain();
    }
    else if (submit_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to present images to swap chain");
    }
}

void Surface_VK::recreate_swapchain()
{
    if (window_container->absolute_width() == 0 || window_container->absolute_height() == 0)
        return;

    swapchain = TGpuHandle<SwapchainResource_VK>("test_swapchain", SwapchainResource_VK::CI_Swapchain{
                                                                       .surface            = surface,
                                                                       .previous_swapchain = swapchain,
                                                                   });

    surface_texture = std::make_shared<Texture_VK>(window_container->absolute_width(), window_container->absolute_height(), 1,
                                                   TextureParameter{
                                                       .format                 = engine_texture_format_from_vk(get_surface_format().format),
                                                       .image_type             = EImageType::Texture_2D,
                                                       .transfer_capabilities  = ETextureTransferCapabilities::None,
                                                       .gpu_write_capabilities = ETextureGPUWriteCapabilities::Enabled,
                                                       .gpu_read_capabilities  = ETextureGPUReadCapabilities::None,
                                                       .mip_level              = 1,
                                                       .read_only              = false,
                                                   },
                                                   swapchain->get_swapchain_images());

    if (main_render_pass)
        main_render_pass->resize(get_container()->absolute_width(), get_container()->absolute_height(), std::vector{surface_texture});
    else
    {
        if (!RenderPassID::exists("resolve_pass"))
        {
            RenderPass::declare_internal(
                RenderPass::Config{
                    .pass_name         = "resolve_pass",
                    .color_attachments = {RenderPass::Config::Attachment{
                        .attachment_name = "color",
                        .image_format    = engine_texture_format_from_vk(get_surface_format().format),
                    }},
                },
                true);
        }

        main_render_pass = RenderPassInstance::create(get_container()->absolute_width(), get_container()->absolute_height(), RenderPassID::get("resolve_pass"), std::vector{surface_texture});
        on_draw          = &main_render_pass->on_draw_pass;
    }
}
} // namespace gfx::vulkan