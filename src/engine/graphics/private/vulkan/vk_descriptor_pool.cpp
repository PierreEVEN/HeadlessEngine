

#include "vulkan/vk_descriptor_pool.h"

#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_allocator.h"

#include <array>
#include <cpputils/logger.hpp>

namespace gfx::vulkan
{

VkDescriptorPool DescriptorPool_VK::alloc_memory(VkDescriptorSetAllocateInfo& alloc_infos)
{
    std::lock_guard lock(find_pool_lock);
    if (alloc_infos.descriptorSetCount > VK_MAX_DESCRIPTOR_PER_POOL)
    {
        LOG_FATAL("Cannot allocate mor than %d descriptors per pool.", VK_MAX_DESCRIPTOR_PER_POOL);
    }
    for (auto& pool : context_pools)
    {
        if (*pool && pool->has_space_for(alloc_infos.descriptorSetCount))
        {
            auto vk_pool = pool->bind_alloc_infos(alloc_infos);
            return vk_pool;
        }
    }
    LOG_INFO("create new descriptor pool");
    context_pools.push_back(new DescriptorPoolItem(alloc_infos));
    return context_pools[context_pools.size() - 1]->pool;
}

VkDescriptorPool DescriptorPoolItem::bind_alloc_infos(VkDescriptorSetAllocateInfo& allocInfos)
{
    space_left -= allocInfos.descriptorSetCount;
    allocInfos.descriptorPool = pool;
    return pool;
}

DescriptorPool_VK::DescriptorPool_VK()
{
}

DescriptorPool_VK::~DescriptorPool_VK()
{
    LOG_INFO("destroy descriptor pools");
    std::lock_guard<std::mutex> lock(find_pool_lock);
    for (const auto& pool : context_pools)
        delete pool;
}

DescriptorPoolItem::DescriptorPoolItem(VkDescriptorSetAllocateInfo& allocInfos) : pool_thread_id(std::this_thread::get_id())
{
    std::array<VkDescriptorPoolSize, 11> poolSizes;
    poolSizes[0]  = {VK_DESCRIPTOR_TYPE_SAMPLER, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[1]  = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[2]  = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[3]  = {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[4]  = {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[5]  = {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[6]  = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[7]  = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[8]  = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[9]  = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_MAX_DESCRIPTOR_PER_TYPE};
    poolSizes[10] = {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_MAX_DESCRIPTOR_PER_TYPE};
    space_left    = VK_MAX_DESCRIPTOR_PER_POOL;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = VK_MAX_DESCRIPTOR_PER_POOL;

    VK_CHECK(vkCreateDescriptorPool(get_device(), &poolInfo, get_allocator(), &pool), "Failed to create descriptor pool");

    bind_alloc_infos(allocInfos);
}

DescriptorPoolItem::~DescriptorPoolItem()
{
    vkDestroyDescriptorPool(get_device(), pool, get_allocator());
}

DescriptorPoolItem::operator bool() const
{
    return std::this_thread::get_id() == pool_thread_id;
}
} // namespace gfx::vulkan