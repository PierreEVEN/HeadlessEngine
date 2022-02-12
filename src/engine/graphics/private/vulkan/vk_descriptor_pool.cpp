

#include "vulkan/vk_descriptor_pool.h"

namespace gfx::vulkan
{
TGpuHandle<DescriptorSetResource_VK> DescriptorPoolManager::new_descriptor_set(const TGpuHandle<DescriptorSetLayoutResource_VK>& layout)
{
    TGpuHandle<DescriptorPoolResource_VK> selected_pool;
    // Find or create a pool than can allocate a descriptor set in the current context
    for (const auto& pool : context_pools)
    {
        if (pool)
        {
            selected_pool = pool;
            break;
        }
    }
    if (!selected_pool)
        selected_pool = TGpuHandle<DescriptorPoolResource_VK>(stringutils::format("descriptor_pool:%s", pool_name.c_str()), DescriptorPoolResource_VK::CreateInfos{});

    return TGpuHandle<DescriptorSetResource_VK>(stringutils::format("descriptor_set:pool=%s", pool_name.c_str()), DescriptorSetResource_VK::CreateInfos{
                                                                      .desc_set_layout = layout,
                                                                      .descriptor_pool = selected_pool,
                                                                  });
}
} // namespace gfx::vulkan