#pragma once
#include "gfx/sampler.h"

#include <vulkan/vulkan.h>
#include <vulkan/vk_unit.h>

namespace gfx::vulkan
{
class Sampler_VK final : public Sampler
{
public:
    Sampler_VK(const std::string& sampler_name, const SamplerOptions& options);
  ~Sampler_VK() override;

    [[nodiscard]] const VkDescriptorImageInfo& get_descriptor_sampler_infos() const
    {
        return sampler_descriptor_infos;
    }

  private:
    VkSampler sampler;
    VkDescriptorImageInfo sampler_descriptor_infos;
};
}
