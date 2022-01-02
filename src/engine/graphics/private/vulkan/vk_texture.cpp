#include "vulkan/vk_texture.h"

#include "assertion.h"
#include "gfx/buffer.h"
#include "one_time_command_buffer.h"
#include "vulkan/allocator.h"
#include "vulkan/device.h"

namespace gfx
{
struct TextureParameter;
}

namespace gfx::vulkan
{

static VkImageUsageFlags vk_usage(const TextureParameter& texture_parameters)
{
    VkImageUsageFlags usage_flags = 0;
    if (static_cast<int>(texture_parameters.transfer_capabilities) & static_cast<int>(ETextureTransferCapabilities::CopySource))
        usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (static_cast<int>(texture_parameters.transfer_capabilities) & static_cast<int>(ETextureTransferCapabilities::CopyDestination))
        usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (texture_parameters.gpu_write_capabilities == ETextureGPUWriteCapabilities::Enabled)
        usage_flags |= Texture::is_depth_format(texture_parameters.format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (texture_parameters.gpu_read_capabilities == ETextureGPUReadCapabilities::Sampling)
        usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

    return usage_flags;
}

VkTexture::VkTexture(uint32_t pixel_width, uint32_t pixel_height, uint32_t pixel_depth, const TextureParameter& parameters) : Texture(pixel_width, pixel_height, pixel_depth, parameters), use_external_images(false)
{
    VkImageCreateInfo image_infos{
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .format        = vk_texture_format_to_engine(image_parameters.format),
        .mipLevels     = image_parameters.mip_level.value(),
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = vk_usage(image_parameters),
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    switch (image_parameters.image_type)
    {
    case EImageType::Texture_1D:
        image_infos.imageType = VK_IMAGE_TYPE_1D;
        image_infos.extent    = {
            .width  = width,
            .height = 1,
            .depth  = 1,
        };
        image_infos.arrayLayers = 1;
        break;
    case EImageType::Texture_1D_Array:
        image_infos.imageType = VK_IMAGE_TYPE_1D;
        image_infos.extent    = {
            .width  = width,
            .height = 1,
            .depth  = 1,
        };
        image_infos.arrayLayers = depth;
        break;
    case EImageType::Texture_2D:
        image_infos.imageType = VK_IMAGE_TYPE_2D;
        image_infos.extent    = {
            .width  = width,
            .height = height,
            .depth  = 1,
        };
        image_infos.arrayLayers = 1;
        break;
    case EImageType::Texture_2D_Array:
        image_infos.imageType = VK_IMAGE_TYPE_2D;
        image_infos.extent    = {
            .width  = width,
            .height = height,
            .depth  = 1,
        };
        image_infos.arrayLayers = depth;
        break;
    case EImageType::Texture_3D:
        image_infos.imageType = VK_IMAGE_TYPE_3D;
        image_infos.extent    = {
            .width  = width,
            .height = height,
            .depth  = depth,
        };
        image_infos.arrayLayers = 1;
        break;
    case EImageType::Cubemap:
        image_infos.imageType = VK_IMAGE_TYPE_2D;
        image_infos.extent    = {
            .width  = width,
            .height = height,
            .depth  = 1,
        };
        image_infos.arrayLayers = 6;
        break;
    }

    const VmaAllocationCreateInfo vma_allocation{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    if (parameters.read_only)
    {
        image_layout = SwapchainImageResource<VkImageLayout>::make_static();
        images       = SwapchainImageResource<VkImage>::make_static();
        allocation   = SwapchainImageResource<VmaAllocation>::make_static();
        view         = SwapchainImageResource<VkImageView>::make_static();
    }

    for (uint8_t i = 0; i < images.get_max_instance_count(); ++i)
    {
        VK_CHECK(vmaCreateImage(get_vma_allocator(), &image_infos, &vma_allocation, &images[i], &allocation[i], nullptr), "failed to create images");
        image_layout[i] = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    create_views();
}

VkTexture::VkTexture(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters, SwapchainImageResource<VkImage>& existing_images)
    : Texture(width, height, depth, parameters), use_external_images(true), images(existing_images)
{
    for (uint8_t i = 0; i < images.get_max_instance_count(); ++i)
        image_layout[i] = VK_IMAGE_LAYOUT_UNDEFINED;

    create_views();
}

void VkTexture::set_pixels(const std::vector<uint8_t>& data)
{
    if (data.size() != get_data_size())
        LOG_FATAL("wrong texture data size ; %d expected %d", data.size(), get_data_size());

    const VkBufferCreateInfo create_infos{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = data.size(),
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };

    const VmaAllocationCreateInfo alloc_info{
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    VkBuffer          staging_buffer;
    VmaAllocation     staging_allocation;
    VmaAllocationInfo allocation_info;

    VK_CHECK(vmaCreateBuffer(get_vma_allocator(), &create_infos, &alloc_info, &staging_buffer, &staging_allocation, &allocation_info), "failed to create vma buffer");
    std::memcpy(allocation_info.pMappedData, data.data(), data.size());

    std::unique_ptr<OneTimeCommandBuffer> copy_command_buffer = std::make_unique<OneTimeCommandBuffer>();

    *image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    update_image_layout(**copy_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    const VkBufferImageCopy region = {
        .bufferOffset      = 0,
        .bufferRowLength   = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask     = is_depth_format(image_parameters.format) ? static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT) : static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT),
                .mipLevel       = 0,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1},
    };
    vkCmdCopyBufferToImage(**copy_command_buffer, staging_buffer, *images, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    update_image_layout(**copy_command_buffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    copy_command_buffer = nullptr;
    vmaDestroyBuffer(get_vma_allocator(), staging_buffer, staging_allocation);
}

void VkTexture::update_image_layout(VkCommandBuffer command_buffer, VkImageLayout new_layout)
{
    VkImageMemoryBarrier barrier = VkImageMemoryBarrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout           = *image_layout,
        .newLayout           = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = *images,
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask     = is_depth_format(image_parameters.format) ? static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT) : static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT),
                .baseMipLevel   = 0,
                .levelCount     = image_parameters.mip_level.value(),
                .baseArrayLayer = 0,
                .layerCount     = image_parameters.image_type == EImageType::Cubemap ? 6u : 1u,
            },
    };
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (*image_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (*image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        LOG_FATAL("Unsupported layout transition : from %d to %d", image_layout, new_layout);
    }

    *image_layout = new_layout;

    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VkTexture::create_views()
{
    VkImageViewCreateInfo image_view_infos{
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .format     = vk_texture_format_to_engine(image_parameters.format),
        .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
        .subresourceRange =
            {
                .aspectMask     = is_depth_format(image_parameters.format) ? static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT) : static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT),
                .baseMipLevel   = 0,
                .levelCount     = image_parameters.mip_level.value(),
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
    };

    switch (image_parameters.image_type)
    {
    case EImageType::Texture_1D:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_1D;
        break;
    case EImageType::Texture_1D_Array:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        break;
    case EImageType::Texture_2D:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case EImageType::Texture_2D_Array:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        break;
    case EImageType::Texture_3D:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case EImageType::Cubemap:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        break;
    }
    for (uint8_t i = 0; i < images.get_max_instance_count(); ++i)
    {
        image_view_infos.image = images[i];
        vkCreateImageView(get_device(), &image_view_infos, get_allocator(), &view[i]);
    }
}

VkTexture ::~VkTexture()
{
    for (uint8_t i = 0; i < images.get_max_instance_count(); ++i)
    {
        vmaDestroyImage(get_vma_allocator(), images[i], allocation[i]);
    }
}
} // namespace gfx::vulkan
