#include "vk_physical_device.h"

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

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice device)
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

    physical_device = device;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    uint32_t queue_index = 0;
    for (const auto& queue : queue_families)
    {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphic_queues.emplace_back(QueueInfo{
                .queue_family  = EQueueFamilyType::GRAPHIC_QUEUE,
                .queue_index = queue_index,
                .queues        = VK_NULL_HANDLE,
            });
        if (queue.queueFlags & VK_QUEUE_COMPUTE_BIT)
            compute_queues.emplace_back(QueueInfo{
                .queue_family  = EQueueFamilyType::COMPUTE_QUEUE,
                .queue_index = queue_index,
                .queues        = VK_NULL_HANDLE,
            });
        if (queue.queueFlags & VK_QUEUE_TRANSFER_BIT)
            transfer_queues.emplace_back(QueueInfo{
                .queue_family  = EQueueFamilyType::TRANSFER_QUEUE,
                .queue_index = queue_index,
                .queues        = VK_NULL_HANDLE,
            });
        queue_index++;
    }
}

VkPhysicalDevice get_physical_device()
{
    if (physical_device == VK_NULL_HANDLE)
        LOG_FATAL("physical device must be initialized first");
    return physical_device;
}

std::vector<QueueInfo*> VulkanPhysicalDevice::get_queues()
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

bool VulkanPhysicalDevice::is_queue_family_supported(EQueueFamilyType queue_family) const
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

QueueInfo VulkanPhysicalDevice::get_queue_family(EQueueFamilyType queue_family, uint8_t queue_index) const
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
