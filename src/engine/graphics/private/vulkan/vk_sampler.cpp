#include "vulkan/vk_sampler.h"

#include "vk_allocator.h"
#include "vk_device.h"
#include "vk_errors.h"
#include "vulkan/vk_helper.h"

namespace gfx::vulkan
{
Sampler_VK::Sampler_VK(const std::string& sampler_name, const SamplerOptions& options) : Sampler(options)
{
    const VkSamplerCreateInfo create_infos{
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .magFilter               = VK_FILTER_LINEAR,
        .minFilter               = VK_FILTER_LINEAR,
        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias              = 0.f,
        .anisotropyEnable        = VK_TRUE,
        .maxAnisotropy           = 16,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .minLod                  = 0.0f,
        .maxLod                  = 0.0f,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    VK_CHECK(vkCreateSampler(get_device(), &create_infos, get_allocator(), &sampler), "failed to create sampler");
    debug_set_object_name(sampler_name, sampler);
    sampler_descriptor_infos = {
        .sampler     = sampler,
        .imageView   = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}

Sampler_VK::~Sampler_VK()
{
    vkDeviceWaitIdle(get_device());
    vkDestroySampler(get_device(), sampler, get_allocator());
}
} // namespace gfx::vulkan
