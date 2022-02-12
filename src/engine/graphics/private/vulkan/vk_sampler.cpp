#include "vulkan/vk_sampler.h"

#include "resources/vk_resource_texture.h"
#include "vk_allocator.h"
#include "vk_device.h"
#include "vk_errors.h"
#include "vulkan/vk_helper.h"

namespace gfx::vulkan
{
Sampler_VK::Sampler_VK(const std::string& sampler_name, const SamplerOptions& options) : Sampler(options)
{
    sampler = TGpuHandle<SamplerResource_VK>(sampler_name, SamplerResource_VK::CreateInfos{});
}
} // namespace gfx::vulkan
