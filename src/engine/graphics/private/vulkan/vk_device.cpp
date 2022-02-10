
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "gfx/resource/gpu_resource.h"
#include "vulkan/vk_device.h"

#include "types/magic_enum.h"
#include "vk_buffer.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_helper.h"
#include "vk_instance.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_config.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_physical_device.h"

#include <cpputils/logger.hpp>
#include <set>

namespace gfx::vulkan
{
FenceResource_VK::FenceResource_VK(const std::string& name, const CI_Fence& create_infos)
{
    const VkFenceCreateInfo fence_infos{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags =( create_infos.signaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<VkFenceCreateFlags>(0)),
    };
    vkCreateFence(get_device(), &fence_infos, get_allocator(), &fence);
    debug_set_object_name(name, fence);
}

FenceResource_VK::~FenceResource_VK()
{
    vkDestroyFence(get_device(), fence, get_allocator());
}

void FenceResource_VK::wait_fence() const
{
    vkWaitForFences(get_device(), 1, &fence, VK_TRUE, UINT64_MAX);
}

SemaphoreResource_VK::SemaphoreResource_VK(const std::string& name, [[maybe_unused]] const CI_Semaphore& create_infos)
{
    const VkSemaphoreCreateInfo semaphore_infos{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    vkCreateSemaphore(get_device(), &semaphore_infos, get_allocator(), &semaphore);
    debug_set_object_name(name, semaphore);
}

SemaphoreResource_VK::~SemaphoreResource_VK()
{
    vkDestroySemaphore(get_device(), semaphore, get_allocator());
}

void Device_VK::init()
{
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
    float                                queue_priorities   = 1.0f;

    get_physical_device<PhysicalDevice_VK>()->update_queues();
    const auto queues = get_physical_device<PhysicalDevice_VK>()->get_queues();

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

    VK_CHECK(vkCreateDevice(GET_VK_PHYSICAL_DEVICE(), &create_infos, get_allocator(), &device), "failed to create device");
    debug_set_object_name("primary device", device);

    std::unordered_map<uint32_t, SwapchainImageResource<TGpuHandle<FenceResource_VK>>> queue_fence_map;
    for (const auto& queue : queues)
    {
        queue_fence_map[queue->queue_index] = {};
        vkGetDeviceQueue(device, queue->queue_index, 0, &queue->queues);
    }

    for (auto& [index, fence] : queue_fence_map)
        for (auto& item : fence)
            item = TGpuHandle<FenceResource_VK>("test fence", FenceResource_VK::CI_Fence{});

    for (const auto& queue : queues)
        queue->queue_submit_fence = queue_fence_map[queue->queue_index];

    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = GET_VK_PHYSICAL_DEVICE(),
        .device         = get_device(),
        .instance       = get_instance(),
    };
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vulkan_memory_allocator), "failed to create vma allocator");
}

Device_VK::~Device_VK()
{
    for (const auto& queue : get_physical_device<PhysicalDevice_VK>()->get_queues())
        queue->queue_submit_fence = {};

    free_allocations();

    command_pool::destroy_pools();

    const auto queues = get_physical_device<PhysicalDevice_VK>()->get_queues();

    VmaStats stats;
    vulkan_memory_allocator->CalculateStats(&stats);
    if (stats.total.allocationCount > 0)
        LOG_FATAL("%d allocation were not destroyed", stats.total.allocationCount);
    vmaDestroyAllocator(vulkan_memory_allocator);

    vkDestroyDevice(device, get_allocator());
    device = VK_NULL_HANDLE;
}

void Device_VK::wait_device()
{
    if (device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(device);
}

} // namespace gfx::vulkan