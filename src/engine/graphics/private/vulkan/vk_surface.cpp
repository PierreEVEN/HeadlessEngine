
#include "vk_surface.h"

#include "application/application.h"
#include "vk_helper.h"
#include "vk_render_pass.h"
#include "vk_render_pass_instance.h"
#include "vk_types.h"
#include "vulkan/vk_command_buffer.h"
#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_texture.h"

#include "vulkan/vk_errors.h"
#include "vulkan/vk_unit.h"

namespace gfx::vulkan
{
Surface_VK::Surface_VK(const std::string& name, application::window::Window* container) : Surface(name, container)
{
    surface = TGpuHandle<SurfaceResource_VK>(stringutils::format("surface:%s", name.c_str()), SurfaceResource_VK::CI_Surface{
                                                                                                  .container = window_container,
                                                                                              });

    for (auto& resource : swapchain_resources)
        resource.image_acquire_semaphore = TGpuHandle<SemaphoreResource_VK>(stringutils::format("semaphore_image_acquire:surface=%s", name.c_str()), SemaphoreResource_VK::CI_Semaphore{});

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

    swapchain = TGpuHandle<SwapchainResource_VK>(stringutils::format("swapchain:surface=%s", surface_name.c_str()), SwapchainResource_VK::CI_Swapchain{
                                                                       .previous_swapchain = swapchain,
                                                                       .surface            = surface,
                                                                   });

    surface_texture = std::make_shared<Texture_VK>("swapchain_image", window_container->absolute_width(), window_container->absolute_height(), 1,
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