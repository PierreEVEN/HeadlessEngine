#include "vulkan/vk_device.h"

#include "types/magic_enum.h"
#include "vk_command_pool.h"
#include "vk_helper.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_config.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_physical_device.h"

#include <cpputils/logger.hpp>
#include <set>

namespace gfx::vulkan
{

static VkDevice logical_device = nullptr;

namespace device
{
void create()
{
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
    float                                queue_priorities   = 1.0f;
    const auto                           queues             = get_physical_device<PhysicalDevice_VK>()->get_queues();

    std::set<int> queue_indices = {};
    for (const auto queue : queues)
        queue_indices.insert(queue->queue_index);

    for (const auto queue : queue_indices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queue;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queue_priorities;
        queue_create_infos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures device_features
    {
#if ENABLE_VALIDATION_LAYER
        .robustBufferAccess = VK_TRUE,
#endif
        .geometryShader       = VK_TRUE,
        .sampleRateShading    = VK_TRUE, // Sample Shading
            .fillModeNonSolid = VK_TRUE, // Wireframe
            .wideLines = VK_TRUE, .samplerAnisotropy = VK_TRUE,
    };

    VkDeviceCreateInfo create_infos = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = NULL,
        .queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos       = queue_create_infos.data(),
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = static_cast<uint32_t>(config::device_extensions.size()),
        .ppEnabledExtensionNames = config::device_extensions.begin(),
        .pEnabledFeatures        = &device_features,
    };

    VK_CHECK(vkCreateDevice(GET_VK_PHYSICAL_DEVICE(), &create_infos, get_allocator(), &logical_device), "failed to create device");
    debug_set_object_name("primary device", logical_device);

    std::unordered_map<uint32_t, SwapchainImageResource<VkFence>> queue_fence_map;
    for (const auto& queue : queues)
    {
        queue_fence_map[queue->queue_index] = {};
        vkGetDeviceQueue(logical_device, queue->queue_index, 0, &queue->queues);
    }

    VkFenceCreateInfo fence_infos{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (auto& [index, fence] : queue_fence_map)
    {
        uint8_t i = 0;
        for (auto& item : fence)
        {
            VK_CHECK(vkCreateFence(get_device(), &fence_infos, get_allocator(), &item), "Failed to create fence");
            debug_set_object_name(stringutils::format("fence queue submit : queue=%d #%d", index, i++), item);
        }
    }

    for (const auto& queue : queues)
        queue->queue_submit_fence = queue_fence_map[queue->queue_index];
}

void destroy()
{
    command_pool::destroy_pools();

    const auto queues = get_physical_device<PhysicalDevice_VK>()->get_queues();

    std::unordered_map<uint32_t, SwapchainImageResource<VkFence>> queue_fence_map;
    for (const auto& queue : queues)
        queue_fence_map[queue->queue_index] = queue->queue_submit_fence;

    for (auto& [index, fence] : queue_fence_map)
        for (auto& item : fence)
            vkDestroyFence(get_device(), item, get_allocator());

    vkDestroyDevice(logical_device, get_allocator());
}
} // namespace device

VkDevice get_device()
{
    if (!logical_device)
        LOG_FATAL("logical device must be initialized first");

    return logical_device;
}
} // namespace gfx::vulkan