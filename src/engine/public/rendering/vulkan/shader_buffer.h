#pragma once
#include "types/nonCopiable.h"

#include <vulkan/vulkan.h>

class ShaderBufferResource final : public NonCopiable
{
  public:
    ShaderBufferResource(const VkBufferUsageFlags& in_buffer_usage);
    ~ShaderBufferResource();

    void set_data(void* data, size_t data_size);
    void resize_buffer(size_t data_size);

    [[nodiscard]] VkDescriptorBufferInfo* get_descriptor_buffer_info();

  private:
    const VkBufferUsageFlags buffer_usage;

    VkBuffer       gpu_buffer    = VK_NULL_HANDLE;
    VkDeviceMemory buffer_memory = VK_NULL_HANDLE;

    VkDescriptorBufferInfo descriptor_buffer_info = {};
};
