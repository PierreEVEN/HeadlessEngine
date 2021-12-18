#pragma once

#include "gfx/physical_device.h"

#include <vulkan/vulkan.hpp>

#define GET_VK_PHYSICAL_DEVICE() (get_physical_device<VulkanPhysicalDevice>()->get_handle())

namespace gfx::vulkan
{
enum class EQueueFamilyType
{
    UNKNOWN        = 0,
    GRAPHIC_QUEUE  = 1,
    COMPUTE_QUEUE  = 2,
    TRANSFER_QUEUE = 3,
};

struct QueueInfo
{
    EQueueFamilyType queue_family  = EQueueFamilyType::UNKNOWN;
    uint32_t         queue_index = 0;
    VkQueue          queues        = VK_NULL_HANDLE;
};

class VulkanPhysicalDevice : public PhysicalDevice
{
  public:
    VulkanPhysicalDevice(VkPhysicalDevice device);

    [[nodiscard]] VkPhysicalDevice get_handle() const
    {
        return physical_device;
    }

    [[nodiscard]] std::vector<QueueInfo*> get_queues();

    [[nodiscard]] bool is_queue_family_supported(EQueueFamilyType queue_family) const;

    [[nodiscard]] QueueInfo get_queue_family(EQueueFamilyType queue_family, uint8_t queue_index = 0) const;

  private:
    VkPhysicalDevice       physical_device;
    std::vector<QueueInfo> graphic_queues;
    std::vector<QueueInfo> transfer_queues;
    std::vector<QueueInfo> compute_queues;
};
} // namespace gfx::vulkan