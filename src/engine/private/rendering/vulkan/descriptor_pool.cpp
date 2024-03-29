

#include "rendering/vulkan/descriptor_pool.h"

#include "config.h"
#include "rendering/graphics.h"
#include "rendering/vulkan/common.h"

#include <array>
#include <cpputils/logger.hpp>

VkDescriptorPool DescriptorPool::alloc_memory(VkDescriptorSetAllocateInfo& alloc_infos)
{
    std::lock_guard<std::mutex> lock(find_pool_lock);
    if (alloc_infos.descriptorSetCount > config::max_descriptor_per_pool)
    {
        LOG_FATAL("Cannot allocate mor than %d descriptors per pool.", config::max_descriptor_per_pool);
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

DescriptorPool::DescriptorPool()
{
}

DescriptorPool::~DescriptorPool()
{
    LOG_INFO("destroy descriptor pools");
    std::lock_guard<std::mutex> lock(find_pool_lock);
    for (auto& pool : context_pools)
        delete pool;
}

DescriptorPoolItem::DescriptorPoolItem(VkDescriptorSetAllocateInfo& allocInfos) : pool_thread_id(std::this_thread::get_id())
{
    std::array<VkDescriptorPoolSize, 11> poolSizes;
    poolSizes[0]  = {VK_DESCRIPTOR_TYPE_SAMPLER, config::max_descriptor_per_type};
    poolSizes[1]  = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, config::max_descriptor_per_type};
    poolSizes[2]  = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, config::max_descriptor_per_type};
    poolSizes[3]  = {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, config::max_descriptor_per_type};
    poolSizes[4]  = {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, config::max_descriptor_per_type};
    poolSizes[5]  = {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, config::max_descriptor_per_type};
    poolSizes[6]  = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, config::max_descriptor_per_type};
    poolSizes[7]  = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, config::max_descriptor_per_type};
    poolSizes[8]  = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, config::max_descriptor_per_type};
    poolSizes[9]  = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, config::max_descriptor_per_type};
    poolSizes[10] = {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, config::max_descriptor_per_type};
    space_left    = config::max_descriptor_per_pool;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = config::max_descriptor_per_pool;

    VK_ENSURE(vkCreateDescriptorPool(Graphics::get()->get_logical_device(), &poolInfo, vulkan_common::allocation_callback, &pool), "Failed to create descriptor pool");

    bind_alloc_infos(allocInfos);
}

DescriptorPoolItem::~DescriptorPoolItem()
{
    vkDestroyDescriptorPool(Graphics::get()->get_logical_device(), pool, vulkan_common::allocation_callback);
}

DescriptorPoolItem::operator bool() const
{
    return std::this_thread::get_id() == pool_thread_id;
}
