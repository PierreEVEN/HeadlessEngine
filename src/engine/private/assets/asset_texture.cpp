

#include "assets/asset_texture.h"
#include <cmath>

#include "rendering/graphics.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/texture.h"
#include "rendering/vulkan/utils.h"

VkDescriptorImageInfo* ATexture::get_descriptor_image_info(uint32_t image_index)
{
    if (descriptor_image_infos.empty() || dirty_descriptors[image_index])
    {
        const auto image_count = Graphics::get()->get_swapchain_config()->get_image_count();
        descriptor_image_infos.resize(image_count);
        if (dirty_descriptors.empty())
            dirty_descriptors.resize(image_count, false);

        dirty_descriptors[image_index] = false;

        for (auto& descriptor : descriptor_image_infos)
        {
            descriptor = VkDescriptorImageInfo{
                .sampler     = get_sampler(image_index),
                .imageView   = get_view(image_index),
                .imageLayout = get_image_layout(image_index),
            };
        }
    }

    return &descriptor_image_infos[image_index];
}

ATexture2D::ATexture2D(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint8_t in_channels)
{
    channels = 4; // = in_channels; @TODO handle channels properly

    switch (channels)
    {
    case 1:
        format = VK_FORMAT_R8_SRGB;
        break;
    case 2:
        format = VK_FORMAT_R8G8_SRGB;
        break;
    case 3:
        format = VK_FORMAT_R8G8B8_SRGB;
        break;
    case 4:
        format = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    default:
        LOG_ERROR("failed to define image format");
    }

    data_size = width * height * channels;

    create_image_and_view(data, width, height, in_channels);
    create_sampler();
}

ATexture2D::~ATexture2D()
{
    if (sampler != VK_NULL_HANDLE)
        vkDestroySampler(Graphics::get()->get_logical_device(), sampler, vulkan_common::allocation_callback);
    if (view != VK_NULL_HANDLE)
        vkDestroyImageView(Graphics::get()->get_logical_device(), view, vulkan_common::allocation_callback);

    if (image != VK_NULL_HANDLE)
        vkDestroyImage(Graphics::get()->get_logical_device(), image, vulkan_common::allocation_callback);
    if (memory != VK_NULL_HANDLE)
        vkFreeMemory(Graphics::get()->get_logical_device(), memory, vulkan_common::allocation_callback);

    /*
    if (uiDisplayLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(G_LOGICAL_DEVICE, uiDisplayLayout, G_ALLOCATION_CALLBACK);
        */

    sampler = VK_NULL_HANDLE;
    view    = VK_NULL_HANDLE;
    image   = VK_NULL_HANDLE;
    memory  = VK_NULL_HANDLE;
    // uiDisplayLayout    = VK_NULL_HANDLE;
}

void ATexture2D::create_image_and_view(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint8_t in_channels)
{

    mips_levels = static_cast<uint32_t>(std::floor(log2(std::max(width, height)))) + 1;

    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    vulkan_utils::create_buffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* raw_data;
    vkMapMemory(Graphics::get()->get_logical_device(), stagingBufferMemory, 0, data_size, 0, &raw_data);
    memcpy(raw_data, data.data(), static_cast<size_t>(data_size));
    vkUnmapMemory(Graphics::get()->get_logical_device(), stagingBufferMemory);

    vulkan_texture::create_image_2d(width, height, mips_levels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);

    auto cmd_buffer = vulkan_utils::begin_single_time_commands();

    vulkan_texture::transition_image_layout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mips_levels, cmd_buffer);
    vulkan_texture::copy_buffer_to_image(stagingBuffer, image, width, height, cmd_buffer);
    vulkan_texture::generate_mipmaps(image, format, width, height, mips_levels, cmd_buffer);
    vulkan_utils::end_single_time_commands(cmd_buffer);

    vulkan_texture::create_image_view_2d(image, view, format, VK_IMAGE_ASPECT_COLOR_BIT, mips_levels);

    vkDestroyBuffer(Graphics::get()->get_logical_device(), stagingBuffer, vulkan_common::allocation_callback);
    vkFreeMemory(Graphics::get()->get_logical_device(), stagingBufferMemory, vulkan_common::allocation_callback);
}

void ATexture2D::create_sampler()
{
    VkSamplerCreateInfo sampler_infos{
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter               = VK_FILTER_LINEAR,
        .minFilter               = VK_FILTER_LINEAR,
        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias              = 0.0f, // Optional
        .anisotropyEnable        = VK_TRUE,
        .maxAnisotropy           = 16.0f,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .minLod                  = 0.0f,
        .maxLod                  = static_cast<float>(mips_levels),
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VK_ENSURE(vkCreateSampler(Graphics::get()->get_logical_device(), &sampler_infos, vulkan_common::allocation_callback, &sampler), "failed to create sampler");
}
