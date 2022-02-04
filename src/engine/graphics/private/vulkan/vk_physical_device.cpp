#include "vk_physical_device.h"

#include "vk_errors.h"
#include "vk_helper.h"
#include "vulkan/vk_device.h"
#include <types/magic_enum.h>
#include <cpputils/logger.hpp>

namespace gfx::vulkan
{
static VkPhysicalDevice physical_device = VK_NULL_HANDLE;

static EPhysicalDeviceType vulkan_device_type_to_engine_type(VkPhysicalDeviceType type)
{
    switch (type)
    {
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return EPhysicalDeviceType::UNKNOWN;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return EPhysicalDeviceType::INTEGRATED_GPU;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return EPhysicalDeviceType::DEDICATED_GPU;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return EPhysicalDeviceType::VIRTUAL_GPU;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return EPhysicalDeviceType::CPU;
    default:
        return EPhysicalDeviceType::UNKNOWN;
    }
}

PhysicalDevice_VK::PhysicalDevice_VK(VkPhysicalDevice device) : physical_device(device)
{
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures   device_features;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    api_version    = device_properties.apiVersion;
    driver_version = device_properties.driverVersion;
    vendor_id      = device_properties.vendorID;
    device_id      = device_properties.deviceID;
    device_type    = vulkan_device_type_to_engine_type(device_properties.deviceType);
    device_name    = device_properties.deviceName;
}

VkFence PhysicalDevice_VK::submit_queue(EQueueFamilyType queue_family, const VkSubmitInfo& submit_infos) const
{
    const QueueInfo& queue_info = get_queue_family(queue_family, 0);
    VK_CHECK(vkWaitForFences(get_device(), 1, &*queue_info.queue_submit_fence, VK_TRUE, UINT64_MAX), "failed to wait on fence");
    vkResetFences(get_device(), 1, &*queue_info.queue_submit_fence);
    VK_CHECK(vkQueueSubmit(queue_info.queues, 1, &submit_infos, *queue_info.queue_submit_fence), "failed to submit queue");
    return *queue_info.queue_submit_fence;
}

void PhysicalDevice_VK::update_queues()
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    graphic_queues.clear();
    compute_queues.clear();
    transfer_queues.clear();

    uint32_t queue_index = 0;
    for (const auto& queue : queue_families)
    {
        if (queue.queueFlags & (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT))
        {
            if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphic_queues.emplace_back(QueueInfo{
                    .queue_family       = EQueueFamilyType::GRAPHIC_QUEUE,
                    .queue_index        = queue_index,
                    .queues             = VK_NULL_HANDLE,
                    .queue_submit_fence = {},
                });
            if (queue.queueFlags & VK_QUEUE_COMPUTE_BIT)
                compute_queues.emplace_back(QueueInfo{
                    .queue_family       = EQueueFamilyType::COMPUTE_QUEUE,
                    .queue_index        = queue_index,
                    .queues             = VK_NULL_HANDLE,
                    .queue_submit_fence = {},
                });
            if (queue.queueFlags & VK_QUEUE_TRANSFER_BIT)
                transfer_queues.emplace_back(QueueInfo{
                    .queue_family       = EQueueFamilyType::TRANSFER_QUEUE,
                    .queue_index        = queue_index,
                    .queues             = VK_NULL_HANDLE,
                    .queue_submit_fence = {},
                });
        }
        queue_index++;
    }
}

VkPhysicalDevice get_physical_device()
{
    if (physical_device == VK_NULL_HANDLE)
        LOG_FATAL("physical device must be initialized first");
    return physical_device;
}

std::vector<QueueInfo*> PhysicalDevice_VK::get_queues()
{
    std::vector<QueueInfo*> queues;

    for (auto& queue : graphic_queues)
        queues.emplace_back(&queue);
    for (auto& queue : compute_queues)
        queues.emplace_back(&queue);
    for (auto& queue : transfer_queues)
        queues.emplace_back(&queue);

    return queues;
}

bool PhysicalDevice_VK::is_queue_family_supported(EQueueFamilyType queue_family) const
{
    switch (queue_family)
    {
    case EQueueFamilyType::GRAPHIC_QUEUE:
        return !graphic_queues.empty();
    case EQueueFamilyType::COMPUTE_QUEUE:
        return !compute_queues.empty();
    case EQueueFamilyType::TRANSFER_QUEUE:
        return !transfer_queues.empty();
    default:
        break;
    }
    return false;
}

QueueInfo PhysicalDevice_VK::get_queue_family(EQueueFamilyType queue_family, uint8_t queue_index) const
{
    switch (queue_family)
    {
    case EQueueFamilyType::GRAPHIC_QUEUE:
        return graphic_queues[queue_index];
    case EQueueFamilyType::COMPUTE_QUEUE:
        return compute_queues[queue_index];
    case EQueueFamilyType::TRANSFER_QUEUE:
        return transfer_queues[queue_index];
    default:
        break;
    }
    return {};
}
} // namespace gfx::vulkan
