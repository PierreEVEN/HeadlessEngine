#pragma once
#include "gfx/types.h"

#include <string>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace gfx::vulkan
{
class BufferResource_VK final
{
  public:
    // Buffer
    struct CI_Buffer
    {
        uint32_t      stride = 1;
        uint32_t      count  = 1;
        EBufferUsage  usage  = EBufferUsage::GPU_MEMORY;
        EBufferAccess access = EBufferAccess::DEFAULT;
        EBufferType   type   = EBufferType::IMMUTABLE;
    };

    BufferResource_VK(const std::string& name, const CI_Buffer& create_infos);
    ~BufferResource_VK();

    [[nodiscard]] VkBuffer& get_buffer()
    {
        return buffer;
    }

    [[nodiscard]] VmaAllocation& get_allocation()
    {
        return memory;
    }

    void* map_buffer();
    void  unmap_buffer();

    VkBuffer               buffer           = VK_NULL_HANDLE;
    VmaAllocation          memory           = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor_infos = {};
};

}
