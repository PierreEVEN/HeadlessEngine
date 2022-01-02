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
    void destroy() const;

    [[nodiscard]] VkCommandPool& get();
    
  private:
    VkCommandPool   command_pool = VK_NULL_HANDLE;
};

class Container final
{
  public:
    Container() = default;
    ~Container();

    [[nodiscard]] VkCommandPool& get();

  private:
    robin_hood::unordered_map<std::thread::id, CommandPool> pools;
};

VkCommandPool& get();

void destroy_pools();
} // namespace command_pool
