#pragma once
#pragma once

#include "gfx/resource/gpu_resource.h"
#include "resources/vk_resource_descriptors.h"

#include <mutex>
#include <vector>

namespace gfx::vulkan
{
class DescriptorPoolManager final
{
  public:
    DescriptorPoolManager(const std::string& name) : pool_name(name)
    {
    }

    // Create a new descriptor set (will find or create a descriptor pool than can currently allocate a new descriptor in the current context)
    TGpuHandle<DescriptorSetResource_VK> new_descriptor_set(const TGpuHandle<DescriptorSetLayoutResource_VK>& layout);

  private:
    const std::string                                  pool_name;
    std::vector<TGpuHandle<DescriptorPoolResource_VK>> context_pools;
    std::mutex                                         find_pool_lock;
};
} // namespace gfx::vulkan