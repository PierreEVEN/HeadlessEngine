#include "rendering/vulkan/command_pool.h"
#include "jobSystem/worker.h"
#include "rendering/graphics.h"

#include <cpputils/logger.hpp>

namespace command_pool
{
CommandPool::CommandPool(VkDevice logical_device, uint32_t queue) : pool_logical_device(logical_device)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queue;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    VK_ENSURE(vkCreateCommandPool(logical_device, &poolInfo, vulkan_common::allocation_callback, &commandPool), "Failed to create command pool");
}

void CommandPool::destroy()
{
    vkDestroyCommandPool(pool_logical_device, commandPool, vulkan_common::allocation_callback);
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

Container::Container() : context_logical_device(Graphics::get()->get_logical_device()), context_queue(Graphics::get()->get_queue_family_indices().graphic_family.value())
{
    command_pool_count = job_system::Worker::get_worker_count() + 1; // One for each worker, plus one for the main thread
    LOG_INFO("create command pool for %d workers", command_pool_count);
    command_pools = static_cast<CommandPool*>(std::malloc(command_pool_count * sizeof(CommandPool)));
    for (int i = 0; i < static_cast<int>(command_pool_count); ++i)
    {
        new (command_pools + i) CommandPool(context_logical_device, context_queue);
    }
}

Container::~Container()
{
    LOG_INFO("destroy command pools");
    for (int i = 0; i < command_pool_count; ++i)
    {
        command_pools[i].destroy();
    }
    free(command_pools);
}

VkCommandPool& Container::get()
{
    for (int i = 0; i < command_pool_count; ++i)
    {
        if (CommandPool& pool = command_pools[i])
        {
            return pool.get();
        }
    }
    LOG_FATAL("no command pool is available on current thread");
}

std::unique_ptr<Container> container = nullptr;

VkCommandPool& get()
{
    if (!container)
    {
        container = std::make_unique<Container>();
    }
    return container->get();
}

void destroy_pools()
{
    container = nullptr;
}
} // namespace command_pool