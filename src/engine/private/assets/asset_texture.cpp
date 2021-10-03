

#include "assets/asset_texture.h"

#include "rendering/gfx_context.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/texture.h"
#include "rendering/vulkan/utils.h"

ATexture::ATexture(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint8_t in_channels)
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

ATexture::~ATexture()
{
    if (sampler != VK_NULL_HANDLE)
        vkDestroySampler(GfxContext::get()->logical_device, sampler, vulkan_common::allocation_callback);
    if (view != VK_NULL_HANDLE)
        vkDestroyImageView(GfxContext::get()->logical_device, view, vulkan_common::allocation_callback);

    if (image != VK_NULL_HANDLE)
        vkDestroyImage(GfxContext::get()->logical_device, image, vulkan_common::allocation_callback);
    if (memory != VK_NULL_HANDLE)
        vkFreeMemory(GfxContext::get()->logical_device, memory, vulkan_common::allocation_callback);

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

VkDescriptorImageInfo* ATexture::get_descriptor_image_info(uint32_t image_index)
{
    if (image_index >= descriptor_image_infos.size())
    {
        for (int64_t i = descriptor_image_infos.size(); i <= image_index; ++i)
        {
            descriptor_image_infos.emplace_back(VkDescriptorImageInfo{
                .sampler     = sampler,
                .imageView   = view,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            });
        }
    }
    return &descriptor_image_infos[image_index];
}

void ATexture::create_image_and_view(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint8_t in_channels)
{

    mips_levels = static_cast<uint32_t>(std::floor(log2(std::max(width, height)))) + 1;

    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    vulkan_utils::create_buffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* raw_data;
    vkMapMemory(GfxContext::get()->logical_device, stagingBufferMemory, 0, data_size, 0, &raw_data);
    memcpy(raw_data, data.data(), static_cast<size_t>(data_size));
    vkUnmapMemory(GfxContext::get()->logical_device, stagingBufferMemory);

    vulkan_texture::create_image_2d(width, height, mips_levels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);

    auto cmd_buffer = vulkan_utils::begin_single_time_commands();

    vulkan_texture::transition_image_layout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mips_levels, cmd_buffer);
    vulkan_texture::copy_buffer_to_image(stagingBuffer, image, width, height, cmd_buffer);
    vulkan_texture::generate_mipmaps(image, format, width, height, mips_levels, cmd_buffer);
    vulkan_utils::end_single_time_commands(cmd_buffer);

    vulkan_texture::create_image_view_2d(image, view, format, VK_IMAGE_ASPECT_COLOR_BIT, mips_levels);

    vkDestroyBuffer(GfxContext::get()->logical_device, stagingBuffer, vulkan_common::allocation_callback);
    vkFreeMemory(GfxContext::get()->logical_device, stagingBufferMemory, vulkan_common::allocation_callback);
}

void ATexture::create_sampler()
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

    VK_ENSURE(vkCreateSampler(GfxContext::get()->logical_device, &sampler_infos, vulkan_common::allocation_callback, &sampler), "failed to create sampler");
}
