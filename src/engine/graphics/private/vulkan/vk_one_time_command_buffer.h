#pragma once

#include "vulkan/vk_physical_device.h"

#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class OneTimeCommandBuffer final
{
  public:
    OneTimeCommandBuffer(EQueueFamilyType queue_family = EQueueFamilyType::GRAPHIC_QUEUE);
    ~OneTimeCommandBuffer();

    VkCommandBuffer& operator*()
    {
        return command_buffer;
    }

  private:
    VkCommandBuffer  command_buffer;
    EQueueFamilyType queue;
};
} // namespace gfx::vulkan
