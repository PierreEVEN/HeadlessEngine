#pragma once

#include <thread>

#include <vulkan/vulkan.h>
#include <types/robin_hood_map.h>

namespace gfx::vulkan::command_pool
{
class CommandPool final
{
  public:
    CommandPool();
    void destroy();

    [[nodiscard]] VkCommandPool& get();

    explicit operator bool();

  private:
    bool            is_created  = false;
    VkCommandPool   commandPool = VK_NULL_HANDLE;
    std::thread::id pool_thread_id;
};

class Container final
{
  public:
    Container() = default;
    ~Container() = default;

    [[nodiscard]] VkCommandPool& get();

  private:
    CommandPool* command_pools      = nullptr;
    size_t       command_pool_count = 0;

    robin_hood::unordered_map<std::thread::id, CommandPool> pools;
};

VkCommandPool& get();

void destroy_pools();
} // namespace command_pool
