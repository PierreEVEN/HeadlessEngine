#pragma once
#include "gfx/sampler.h"
#include "resources/vk_resource_texture.h"

#include <vulkan/vk_unit.h>
#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class Sampler_VK final : public Sampler
{
  public:
    Sampler_VK(const std::string& sampler_name, const SamplerOptions& options);
    ~Sampler_VK() override = default;

    [[nodiscard]] const VkDescriptorImageInfo& get_descriptor_sampler_infos() const
    {
        return sampler->sampler_descriptor_infos;
    }

  private:
    TGpuHandle<SamplerResource_VK> sampler;
};
} // namespace gfx::vulkan
