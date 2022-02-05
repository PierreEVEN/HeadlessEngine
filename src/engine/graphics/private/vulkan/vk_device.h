#pragma once

#include "gfx/resource/device.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#define get_device() ((Device_VK*)&Device::get())->get_device_temp_internal_afac()

#define get_vma_allocator() ((Device_VK*)&Device::get())->get_vma_allocator_temp_internal_afac()

namespace gfx::vulkan
{

class QueueResource_VK final
{
  public:
    QueueResource_VK(const std::string& name) {}
    ~QueueResource_VK() = default;

    VkQueue queue = VK_NULL_HANDLE;
};

class Device_VK final : public Device
{
  public:
    void init() override;
    Device_VK(uint8_t image_count) : Device(image_count)
    {
    }
    ~Device_VK() override;

    [[nodiscard]] VkDevice get_device_temp_internal_afac() const
    {
        return device;
    }

    [[nodiscard]] const VmaAllocator& get_vma_allocator_temp_internal_afac() const
    {
        return vulkan_memory_allocator;
    }

    void wait_device() override;

  private:
    VkDevice     device;
    VmaAllocator vulkan_memory_allocator;
};
} // namespace gfx::vulkan