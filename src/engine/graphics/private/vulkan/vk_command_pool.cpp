#include "vulkan/vk_command_pool.h"

#include "gfx/physical_device.h"
#include "vk_helper.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_physical_device.h"

#include <cpputils/logger.hpp>

namespace gfx::vulkan::command_pool
{
CommandPool::CommandPool()
{
    const VkCommandPoolCreateInfo pool_infos = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // Optional
        .queueFamilyIndex = get_physical_device<PhysicalDevice_VK>()->get_queue_family(EQueueFamilyType::GRAPHIC_QUEUE).queue_index,
    };
    VK_CHECK(vkCreateCommandPool(get_device(), &pool_infos, get_allocator(), &command_pool), "Failed to create command pool");
    debug_set_object_name(stringutils::format("command_pool @%x", std::this_thread::get_id()), command_pool);

} // namespace gfx::vulkan::command_pool

void CommandPool::destroy() const
{
    vkDestroyCommandPool(get_device(), command_pool, get_allocator());
}

VkCommandPool& CommandPool::get()
{
    return command_pool;
}

Container::~Container()
{
    for (const auto& [id, pool] : pools)
        pool.destroy();
}

VkCommandPool& Container::get()
{
    auto pool = pools.find(std::this_thread::get_id());
    if (pool == pools.end())
    {
        pools.emplace(std::this_thread::get_id(), CommandPool());
        return pools[std::this_thread::get_id()].get();
    }

    return pools[std::this_thread::get_id()].get();
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
} // namespace gfx::vulkan::command_pool