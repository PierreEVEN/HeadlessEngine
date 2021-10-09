

#include "rendering/vulkan/framebuffer.h"

#include "rendering/graphics.h"
#include "rendering/renderer/render_pass.h"
#include "rendering/renderer/swapchain.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/texture.h"

#include <cpputils/logger.hpp>

AFramebufferImage::AFramebufferImage(Swapchain* in_source_swapchain) : source_swapchain(in_source_swapchain)
{
    create_or_recreate();
}

AFramebufferImage::AFramebufferImage(const FramebufferImageInfo& in_framebuffer_image_info) : source_swapchain(nullptr), framebuffer_image_info(in_framebuffer_image_info)
{
    create_or_recreate();
}

AFramebufferImage::~AFramebufferImage()
{
    delete_resources();
}

void AFramebufferImage::resize_buffer(VkExtent2D in_buffer_size)
{
    Graphics::get()->wait_device();
    framebuffer_image_info.image_extent = in_buffer_size;
    create_or_recreate();
}

void AFramebufferImage::create_or_recreate()
{
    delete_resources();
    if (source_swapchain)
    {
        uint32_t swapchain_image_count = Graphics::get()->get_swapchain_config()->get_image_count();

        images.resize(swapchain_image_count);
        views.resize(swapchain_image_count);
        vkGetSwapchainImagesKHR(Graphics::get()->get_logical_device(), source_swapchain->get_swapchain_khr(), &swapchain_image_count, images.data());
        for (size_t i = 0; i < swapchain_image_count; i++)
            vulkan_texture::create_image_view_2d(images[i], views[i], Graphics::get()->get_swapchain_config()->get_surface_format().format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        images.clear();
    }
    else
    {
        images.resize(framebuffer_image_info.image_count);
        memories.resize(framebuffer_image_info.image_count);
        views.resize(framebuffer_image_info.image_count);
        for (int i = 0; i < framebuffer_image_info.image_count; ++i)
        {
            vulkan_texture::create_image_2d(
                framebuffer_image_info.image_extent.width, framebuffer_image_info.image_extent.height, 1, framebuffer_image_info.sample_count, framebuffer_image_info.image_format, VK_IMAGE_TILING_OPTIMAL,
                framebuffer_image_info.is_depth_stencil ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | (framebuffer_image_info.b_use_sampler ? VK_IMAGE_USAGE_SAMPLED_BIT : 0)
                                                        : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (framebuffer_image_info.b_use_sampler ? VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_TRANSFER_DST_BIT),
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, images[i], memories[i]);
            vulkan_texture::create_image_view_2d(images[i], views[i], framebuffer_image_info.image_format, framebuffer_image_info.is_depth_stencil ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
        if (framebuffer_image_info.b_use_sampler)
            create_sampler();
        image_layout = framebuffer_image_info.is_depth_stencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    mark_descriptor_dirty();
}

void AFramebufferImage::create_sampler()
{
    VkSamplerCreateInfo samplerInfo{
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter               = VK_FILTER_NEAREST,
        .minFilter               = VK_FILTER_NEAREST,
        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias              = 0.0f,
        .anisotropyEnable        = VK_FALSE,
        .maxAnisotropy           = 1.f,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .minLod                  = 0.0f,
        .maxLod                  = 1,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VK_ENSURE(vkCreateSampler(Graphics::get()->get_logical_device(), &samplerInfo, vulkan_common::allocation_callback, &sampler), "failed to create sampler");
}

void AFramebufferImage::delete_resources()
{
    if (sampler != VK_NULL_HANDLE)
        vkDestroySampler(Graphics::get()->get_logical_device(), sampler, vulkan_common::allocation_callback);
    for (const auto& view : views)
        vkDestroyImageView(Graphics::get()->get_logical_device(), view, vulkan_common::allocation_callback);
    for (const auto& image : images)
        vkDestroyImage(Graphics::get()->get_logical_device(), image, vulkan_common::allocation_callback);
    for (const auto& memory : memories)
        vkFreeMemory(Graphics::get()->get_logical_device(), memory, vulkan_common::allocation_callback);
}

Framebuffer::Framebuffer(RenderPass* used_render_pass, VkExtent2D in_buffer_size, Swapchain* current_swapchain) : render_pass(used_render_pass), swapchain(current_swapchain), buffer_size(in_buffer_size)
{
    create_framebuffer_images();
    create_framebuffer();
}

Framebuffer::~Framebuffer()
{
    destroy_framebuffer();
    destroy_framebuffer_images();
}

void Framebuffer::resize_framebuffer(const VkExtent2D& new_extent)
{
    Graphics::get()->wait_device();

    destroy_framebuffer();
    buffer_size = new_extent;

    for (const auto& image : framebuffer_images)
    {
        image->resize_buffer(new_extent);
    }
    create_framebuffer();
}

void Framebuffer::create_framebuffer_images()
{
    const auto& pass_description = render_pass->get_render_pass_description();

    framebuffer_images.clear();

    for (const auto& col_attachment : pass_description.color_attachments)
    {
        const uint32_t attachment_index = static_cast<uint32_t>(framebuffer_images.size());
        if (pass_description.b_use_swapchain_image && !pass_description.has_resolve_attachment()) // if this is the last attachment, use the swapchain images instead of creating new one
        {
            framebuffer_images.emplace_back(AssetManager::get()->create<AFramebufferImage>(stringutils::format("framebuffer_image-%s_%d", pass_description.pass_name.c_str(), attachment_index).c_str(), swapchain));
        }
        else
        {
            const FramebufferImageInfo image_info{
                .image_extent     = buffer_size,
                .sample_count     = pass_description.sample_count,
                .image_format     = col_attachment.image_format,
                .is_depth_stencil = false,
                .image_count      = 1,
                .b_use_sampler    = true,
            };
            framebuffer_images.emplace_back(AssetManager::get()->create<AFramebufferImage>(stringutils::format("framebuffer_image-%s_%d", pass_description.pass_name.c_str(), attachment_index).c_str(), image_info));
        }
    }

    if (pass_description.depth_attachment)
    {
        const uint32_t attachment_index = static_cast<uint32_t>(framebuffer_images.size());

        const FramebufferImageInfo image_info{
            .image_extent     = buffer_size,
            .sample_count     = pass_description.sample_count,
            .image_format     = pass_description.depth_attachment->image_format,
            .is_depth_stencil = true,
            .image_count      = 1,
            .b_use_sampler    = true,
        };
        framebuffer_images.emplace_back(AssetManager::get()->create<AFramebufferImage>(stringutils::format("framebuffer_image-%s_depth", pass_description.pass_name.c_str(), attachment_index).c_str(), image_info));
    }

    if (pass_description.has_resolve_attachment())
    {
        if (pass_description.b_use_swapchain_image)
        {
            framebuffer_images.emplace_back(AssetManager::get()->create<AFramebufferImage>(stringutils::format("framebuffer_image-%s_resolve", pass_description.pass_name.c_str()).c_str(), swapchain));
        }
        else
        {
            LOG_FATAL("NOT IMPLEMENTED YET : @TODO : create custom resolve attachment that doesn't use swapchain image");
        }
    }
}

void Framebuffer::create_framebuffer()
{
    const uint32_t image_count = Graphics::get()->get_swapchain_config()->get_image_count();
    framebuffers.resize(image_count);

    for (size_t i = 0; i < image_count; i++)
    {
        std::vector<VkImageView> attachments(0);
        for (const auto& image : framebuffer_images)
        {
            attachments.emplace_back(image->get_view(static_cast<uint32_t>(i)));
        }

        const VkFramebufferCreateInfo framebuffer_infos{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = render_pass->get_render_pass(),
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments    = attachments.data(),
            .width           = buffer_size.width,
            .height          = buffer_size.height,
            .layers          = 1,
        };

        VK_ENSURE(vkCreateFramebuffer(Graphics::get()->get_logical_device(), &framebuffer_infos, vulkan_common::allocation_callback, &framebuffers[i]), stringutils::format("Failed to create framebuffer #%d", i).c_str());
    }
}

void Framebuffer::destroy_framebuffer_images()
{
    for (auto& image : framebuffer_images)
    {
        if (AssetManager::is_valid())
            AssetManager::get()->remove(&image);
    }
    framebuffer_images.clear();
}

void Framebuffer::destroy_framebuffer()
{
    for (auto framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(Graphics::get()->get_logical_device(), framebuffer, nullptr);
    }
}
