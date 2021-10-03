#pragma once
#include <mutex>
#include <vector>
#include <thread>
#include <vulkan/vulkan_core.h>

class DescriptorPoolItem final
{
  private:
    friend class DescriptorPool;

    static void clear_pools();

    DescriptorPoolItem(VkDescriptorSetAllocateInfo& allocInfos);
    ~DescriptorPoolItem();

    explicit operator bool() const;

    [[nodiscard]] bool has_space_for(const uint32_t& required_space) const
    {
        return space_left >= required_space;
    }

    void bind_alloc_infos(VkDescriptorSetAllocateInfo& allocInfos);

    VkDescriptorPool pool       = VK_NULL_HANDLE;
    uint32_t         space_left = 0;
    std::thread::id  pool_thread_id;
};

class DescriptorPool final
{
  public:
    DescriptorPool();
    ~DescriptorPool();

    void alloc_memory(VkDescriptorSetAllocateInfo& alloc_infos);

  private:
    std::vector<DescriptorPoolItem*> context_pools;
    std::mutex                       find_pool_lock;
};
