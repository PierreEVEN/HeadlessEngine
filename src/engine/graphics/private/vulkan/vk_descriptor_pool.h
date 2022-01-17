#pragma once
#pragma once

#include <mutex>
#include <thread>
#include <vector>
#include <vulkan/vulkan.h>

#ifndef VK_MAX_DESCRIPTOR_PER_POOL
#define VK_MAX_DESCRIPTOR_PER_POOL 64
#endif
#ifndef VK_MAX_DESCRIPTOR_PER_TYPE
#define VK_MAX_DESCRIPTOR_PER_TYPE 256
#endif

namespace gfx::vulkan
{

class DescriptorPoolItem final
{
    friend class DescriptorPool_VK;
  private:

    static void clear_pools();

    DescriptorPoolItem(VkDescriptorSetAllocateInfo& allocInfos);
    ~DescriptorPoolItem();

    explicit operator bool() const;

    [[nodiscard]] bool has_space_for(const uint32_t& required_space) const
    {
        return space_left >= required_space;
    }

    VkDescriptorPool bind_alloc_infos(VkDescriptorSetAllocateInfo& allocInfos);

    VkDescriptorPool pool       = VK_NULL_HANDLE;
    uint32_t         space_left = 0;
    std::thread::id  pool_thread_id;
};

class DescriptorPool_VK final
{
  public:
    DescriptorPool_VK();
    ~DescriptorPool_VK();

    VkDescriptorPool alloc_memory(VkDescriptorSetAllocateInfo& alloc_infos);

  private:
    std::vector<DescriptorPoolItem*> context_pools;
    std::mutex                       find_pool_lock;
};
} // namespace gfx::vulkan