#pragma once
#include "gfx/materials/material_instance.h"

#include <vulkan/vulkan.hpp>
#include "vulkan/vk_unit.h"

namespace gfx::vulkan
{
class MaterialInstance_VK : public MaterialInstance
{
public:
    MaterialInstance_VK(const std::shared_ptr<MasterMaterial>& base);
    void bind_buffer(const std::string& binding_name, Buffer* in_buffer) override;

private:
  void                                                    build_descriptor_sets();
    RenderPassData<SwapchainImageResource<VkDescriptorSet>> descriptor_sets;
};

}
