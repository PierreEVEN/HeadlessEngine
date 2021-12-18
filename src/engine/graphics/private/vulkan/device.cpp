#include "vulkan/device.h"
#include "vulkan/vk_physical_device.h"

#include "vulkan/allocator.h"
#include "vulkan/assertion.h"

#include <set>
#include <vulkan/vulkan.hpp>

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{

static VkDevice logical_device = nullptr;

namespace device
{
void create()
{
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
    float                                queue_priorities   = 1.0f;
    const auto                           queues             = get_physical_device<VulkanPhysicalDevice>()->get_queues();

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

    VkPhysicalDeviceFeatures device_features{
        .geometryShader    = VK_TRUE,
        .sampleRateShading = VK_TRUE, // Sample Shading
        .fillModeNonSolid  = VK_TRUE, // Wireframe
        .wideLines         = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };

    VkDeviceCreateInfo create_infos = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = NULL,
        .queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos       = queue_create_infos.data(),
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures        = &device_features,
    };

    VK_CHECK(vkCreateDevice(GET_VK_PHYSICAL_DEVICE(), &create_infos, get_allocator(), &logical_device), "failed to create device");

    for (const auto& queue : queues)
        vkGetDeviceQueue(logical_device, queue->queue_index, 0, &queue->queues);
}
void destroy()
{
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