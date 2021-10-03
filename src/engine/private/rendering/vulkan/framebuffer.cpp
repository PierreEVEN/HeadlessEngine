

#include "rendering/vulkan/framebuffer.h"

#include "rendering/vulkan/texture.h"
#include "rendering/renderer/surface.h"
#include "rendering/vulkan/common.h"
#include "rendering/gfx_context.h"
#include "rendering/renderer/render_pass.h"

#include <cpputils/logger.hpp>

Framebuffer::Framebuffer(RenderPass* used_render_pass, Swapchain* current_swapchain) : render_pass(used_render_pass), swapchain(current_swapchain)
{
    buffer_size = current_swapchain->get_swapchain_extend();
    create_framebuffer_images();
    create_framebuffer();
}

Framebuffer::~Framebuffer()
{
    destroy_framebuffer();
    destroy_framebuffer_images();
}

void Framebuffer::create_framebuffer_images()
{
    /** Color buffer */
    VkFormat colorFormat = swapchain->get_surface_format().format;
    vulkan_texture::create_image_2d(buffer_size.width, buffer_size.height, 1, static_cast<VkSampleCountFlagBits>(vulkan_common::get_msaa_sample_count()), colorFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, color_target, color_memory);
    vulkan_texture::create_image_view_2d(color_target, color_view, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    /** Depth buffer */
    VkFormat depthFormat = vulkan_utils::get_depth_format(GfxContext::get()->physical_device);
    vulkan_texture::create_image_2d(buffer_size.width, buffer_size.height, 1, static_cast<VkSampleCountFlagBits>(vulkan_common::get_msaa_sample_count()), depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_target, depth_memory);
    vulkan_texture::create_image_view_2d(depth_target, depth_view, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    /** Swap chain buffer */
    uint32_t swapchain_image_count = swapchain->get_image_count();
    swapchain_images.resize(swapchain->get_image_count());
    swapchain_image_views.resize(swapchain->get_image_count());
    vkGetSwapchainImagesKHR(GfxContext::get()->logical_device, swapchain->get_swapchain_khr(), &swapchain_image_count, nullptr);
    vkGetSwapchainImagesKHR(GfxContext::get()->logical_device, swapchain->get_swapchain_khr(), &swapchain_image_count, swapchain_images.data());
    for (size_t i = 0; i < swapchain_images.size(); i++)
    {
        vulkan_texture::create_image_view_2d(swapchain_images[i], swapchain_image_views[i], swapchain->get_surface_format().format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Framebuffer::create_framebuffer()
{
    framebuffers.resize(swapchain->get_image_count());

    for (size_t i = 0; i < swapchain->get_image_count(); i++)
    {
        std::vector<VkImageView> attachments;
        if (vulkan_common::get_msaa_sample_count() > 1)
        {
            attachments.push_back(color_view);
            attachments.push_back(depth_view);
            attachments.push_back(swapchain_image_views[i]);
        }
        else
        {
            attachments.push_back(swapchain_image_views[i]);
            attachments.push_back(depth_view);
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = render_pass->get_render_pass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = buffer_size.width;
        framebufferInfo.height          = buffer_size.height;
        framebufferInfo.layers          = 1;

        VK_ENSURE(vkCreateFramebuffer(GfxContext::get()->logical_device, &framebufferInfo, vulkan_common::allocation_callback, &framebuffers[i]), "Failed to create framebuffer #%d" + i);
    }
}

void Framebuffer::destroy_framebuffer_images()
{
    for (auto framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(GfxContext::get()->logical_device, framebuffer, nullptr);
    }
}

void Framebuffer::destroy_framebuffer()
{
    /** Color buffer */
    vkDestroyImageView(GfxContext::get()->logical_device, color_view, vulkan_common::allocation_callback);
    vkDestroyImage(GfxContext::get()->logical_device, color_target, vulkan_common::allocation_callback);
    vkFreeMemory(GfxContext::get()->logical_device, color_memory, vulkan_common::allocation_callback);
    color_view   = VK_NULL_HANDLE;
    color_target = VK_NULL_HANDLE;
    color_memory = VK_NULL_HANDLE;

    /** Depth buffer */
    vkDestroyImageView(GfxContext::get()->logical_device, depth_view, vulkan_common::allocation_callback);
    vkDestroyImage(GfxContext::get()->logical_device, depth_target, vulkan_common::allocation_callback);
    vkFreeMemory(GfxContext::get()->logical_device, depth_memory, vulkan_common::allocation_callback);
    depth_view   = VK_NULL_HANDLE;
    depth_target = VK_NULL_HANDLE;
    depth_memory = VK_NULL_HANDLE;

    /* Swap chain buffers */
    for (auto imageView : swapchain_image_views)
        vkDestroyImageView(GfxContext::get()->logical_device, imageView, vulkan_common::allocation_callback);

    swapchain_image_views.clear();
}
