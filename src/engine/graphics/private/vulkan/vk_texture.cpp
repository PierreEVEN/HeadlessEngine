#include "vulkan/vk_texture.h"

#include "vk_types.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_one_time_command_buffer.h"

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

Texture_VK::Texture_VK(uint32_t pixel_width, uint32_t pixel_height, uint32_t pixel_depth, const TextureParameter& parameters) : Texture(pixel_width, pixel_height, pixel_depth, parameters)
{
    if (parameters.read_only)
    {
        images = SwapchainImageResource<TGpuHandle<ImageResource_VK>>::make_static();
        views  = SwapchainImageResource<TGpuHandle<ImageViewResource_VK>>::make_static();
    }

    for (uint8_t i = 0; i < images.get_max_instance_count(); ++i)
    {
        images[i] = TGpuHandle<ImageResource_VK>("unknown image", ImageResource_VK::CI_Texture{
                                                                      .width              = width,
                                                                      .height             = height,
                                                                      .depth              = depth,
                                                                      .texture_parameters = parameters,
                                                                  });

        views[i] = TGpuHandle<ImageViewResource_VK>("unknown view", ImageViewResource_VK::CI_TextureView{
                                                                        .texture_parameters = parameters,
                                                                        .used_image         = images[i],
                                                                    });
    }
}

Texture_VK::Texture_VK(uint32_t image_width, uint32_t image_height, uint32_t image_depth, const TextureParameter& parameters, SwapchainImageResource<VkImage>& existing_images)
    : Texture(image_width, image_height, image_depth, parameters)
{
    for (uint8_t i = 0; i < existing_images.get_max_instance_count(); ++i)
    {
        views[i] = TGpuHandle<ImageViewResource_VK>("unknown view", ImageViewResource_VK::CI_TextureView{
                                                                        .texture_parameters = parameters,
                                                                        .used_image         = TGpuHandle<ImageResource_VK>("Unknown existing image",
                                                                                                                   ImageResource_VK::CI_Texture{
                                                                                                                       .width              = width,
                                                                                                                       .height             = height,
                                                                                                                       .depth              = depth,
                                                                                                                       .texture_parameters = parameters,
                                                                                                                   },
                                                                                                                   existing_images[i]),
                                                                    });
    }
}

void Texture_VK::set_pixels(const std::vector<uint8_t>& data)
{
    if (auto& image = *images)
        image->set_pixels(data);
}

ImageResource_VK::ImageResource_VK(const std::string& name, const CI_Texture& create_infos) : image_layout(VK_IMAGE_LAYOUT_UNDEFINED), image_infos(create_infos)
{
    VkImageCreateInfo image_create_infos{
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .format        = vk_texture_format_to_engine(create_infos.texture_parameters.format),
        .mipLevels     = create_infos.texture_parameters.mip_level ? create_infos.texture_parameters.mip_level.value() : 1,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = vk_usage(create_infos.texture_parameters),
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    switch (create_infos.texture_parameters.image_type)
    {
    case EImageType::Texture_1D:
        image_create_infos.imageType = VK_IMAGE_TYPE_1D;
        image_create_infos.extent    = {
            .width  = create_infos.width,
            .height = 1,
            .depth  = 1,
        };
        image_create_infos.arrayLayers = 1;
        break;
    case EImageType::Texture_1D_Array:
        image_create_infos.imageType = VK_IMAGE_TYPE_1D;
        image_create_infos.extent    = {
            .width  = create_infos.width,
            .height = 1,
            .depth  = 1,
        };
        image_create_infos.arrayLayers = create_infos.depth;
        break;
    case EImageType::Texture_2D:
        image_create_infos.imageType = VK_IMAGE_TYPE_2D;
        image_create_infos.extent    = {
            .width  = create_infos.width,
            .height = create_infos.height,
            .depth  = 1,
        };
        image_create_infos.arrayLayers = 1;
        break;
    case EImageType::Texture_2D_Array:
        image_create_infos.imageType = VK_IMAGE_TYPE_2D;
        image_create_infos.extent    = {
            .width  = create_infos.width,
            .height = create_infos.height,
            .depth  = 1,
        };
        image_create_infos.arrayLayers = create_infos.depth;
        break;
    case EImageType::Texture_3D:
        image_create_infos.imageType = VK_IMAGE_TYPE_3D;
        image_create_infos.extent    = {
            .width  = create_infos.width,
            .height = create_infos.height,
            .depth  = create_infos.depth,
        };
        image_create_infos.arrayLayers = 1;
        break;
    case EImageType::Cubemap:
        image_create_infos.imageType = VK_IMAGE_TYPE_2D;
        image_create_infos.extent    = {
            .width  = create_infos.width,
            .height = create_infos.height,
            .depth  = 1,
        };
        image_create_infos.arrayLayers = 6;
        break;
    }
    const VmaAllocationCreateInfo vma_allocation{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };
    vmaCreateImage(get_vma_allocator(), &image_create_infos, &vma_allocation, &image, &memory, nullptr);
    debug_set_object_name(name, image);
}

ImageResource_VK::ImageResource_VK(const std::string& name, const CI_Texture& create_infos, const VkImage& existing_image_handle)
    : image(existing_image_handle), image_layout(VK_IMAGE_LAYOUT_UNDEFINED), memory(VK_NULL_HANDLE), image_infos(create_infos)
{
    debug_set_object_name(name, image);
}

ImageResource_VK::~ImageResource_VK()
{
    if (memory != VK_NULL_HANDLE)
        vmaDestroyImage(get_vma_allocator(), image, memory);
}

void ImageResource_VK::set_image_layout(VkCommandBuffer command_buffer, VkImageLayout new_layout)
{
    VkImageMemoryBarrier barrier = VkImageMemoryBarrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout           = image_layout,
        .newLayout           = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask     = is_depth_format(image_infos.texture_parameters.format) ? static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT) : static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT),
                .baseMipLevel   = 0,
                .levelCount     = image_infos.texture_parameters.mip_level ? image_infos.texture_parameters.mip_level.value() : 1,
                .baseArrayLayer = 0,
                .layerCount     = image_infos.texture_parameters.image_type == EImageType::Cubemap ? 6u : 1u,
            },
    };
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (image_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
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

    image_layout = new_layout;

    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void ImageResource_VK::set_pixels(const std::vector<uint8_t>& data)
{
    if (memory == VK_NULL_HANDLE)
        LOG_FATAL("cannot set data on current texture");

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

    set_image_layout(**copy_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    const VkBufferImageCopy region = {
        .bufferOffset      = 0,
        .bufferRowLength   = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask     = is_depth_format(image_infos.texture_parameters.format) ? static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT) : static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT),
                .mipLevel       = 0,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
        .imageOffset = {0, 0, 0},
        .imageExtent = {image_infos.width, image_infos.height, 1},
    };

    vkCmdCopyBufferToImage(**copy_command_buffer, staging_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    set_image_layout(**copy_command_buffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    copy_command_buffer = nullptr;
    vmaDestroyBuffer(get_vma_allocator(), staging_buffer, staging_allocation);
}

size_t ImageResource_VK::get_data_size()
{
    return image_infos.width * image_infos.height * image_infos.depth * get_format_bytes_per_pixel(image_infos.texture_parameters.format) * get_format_channel_count(image_infos.texture_parameters.format);
}

ImageViewResource_VK::ImageViewResource_VK(const std::string& name, const CI_TextureView& create_infos)
{
    VkImageViewCreateInfo image_view_infos{
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .format     = vk_texture_format_to_engine(create_infos.texture_parameters.format),
        .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
        .subresourceRange =
            {
                .aspectMask     = is_depth_format(create_infos.texture_parameters.format) ? static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT) : static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT),
                .baseMipLevel   = 0,
                .levelCount     = create_infos.texture_parameters.mip_level ? create_infos.texture_parameters.mip_level.value() : 1,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
    };

    switch (create_infos.texture_parameters.image_type)
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
    image_view_infos.image = create_infos.used_image->image;
    vkCreateImageView(get_device(), &image_view_infos, get_allocator(), &view);

    debug_set_object_name(name, view);

    descriptor_infos = VkDescriptorImageInfo{
        .sampler     = VK_NULL_HANDLE,
        .imageView   = view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    LOG_WARNING("create view");
}

ImageViewResource_VK::~ImageViewResource_VK()
{
    vkDestroyImageView(get_device(), view, get_allocator());
}

} // namespace gfx::vulkan
