#include "command_pool.h"

#include "allocator.h"
#include "assertion.h"
#include "device.h"
#include "vk_physical_device.h"
#include "gfx/physical_device.h"

#include <cpputils/logger.hpp>

namespace gfx::vulkan::command_pool
{
CommandPool::CommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = get_physical_device<VulkanPhysicalDevice>()->get_queue_family(EQueueFamilyType::GRAPHIC_QUEUE).queue_index;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    VK_CHECK(vkCreateCommandPool(get_device(), &poolInfo, get_allocator(), &commandPool), "Failed to create command pool");
}

void CommandPool::destroy()
{
    vkDestroyCommandPool(get_device(), commandPool, get_allocator());
}

VkCommandPool& CommandPool::get()
{
    return commandPool;
}

CommandPool::operator bool()
{
    if (!is_created)
    {
        is_created     = true;
        pool_thread_id = std::this_thread::get_id();
        return true;
    }
    return std::this_thread::get_id() == pool_thread_id;
}



VkCommandPool& Container::get()
{
    auto pool = pools.find(std::this_thread::get_id());
    if (pool == pools.end())
    {
        pools.emplace(std::this_thread::get_id(), CommandPool());
        return pools[std::this_thread::get_id()].get();
    }

    robin_hood::pair<std::thread::id, CommandPool>& pair = *pool;
    return pair.second.get();
}

std::unique_ptr<Container> container = nullptr;

VkCommandPool& get()
{
    if (!container)
        container = std::make_unique<Container>();

    return container->get();
}

void destroy_pools()
{
    container = nullptr;
}
} // namespace command_pool