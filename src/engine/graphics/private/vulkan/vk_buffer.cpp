

#include "vk_buffer.h"

#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_errors.h"
#include "types/magic_enum.h"

#include <string>
#include <vulkan/vk_allocator.h>

namespace gfx::vulkan
{
Buffer_VK::Buffer_VK(const std::string& buffer_name, uint32_t buffer_stride, uint32_t elements, EBufferUsage buffer_usage, EBufferAccess in_buffer_access)
    : Buffer(buffer_name, buffer_stride, elements, buffer_usage, in_buffer_access)
{
    VkBufferUsageFlags vk_usage = 0;
    switch (usage)
    {
    case EBufferUsage::INDEX_DATA:
        vk_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case EBufferUsage::VERTEX_DATA:
        vk_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case EBufferUsage::GPU_MEMORY:
        vk_usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    case EBufferUsage::UNIFORM_BUFFER:
        vk_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case EBufferUsage::INDIRECT_DRAW_ARGUMENT:
        vk_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        break;
    case EBufferUsage::TRANSFER_MEMORY:
        vk_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        break;
    }

    VmaMemoryUsage vma_usage = VMA_MEMORY_USAGE_UNKNOWN;

    switch (buffer_access)
    {
    case EBufferAccess::DEFAULT:
        vma_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
    case EBufferAccess::GPU_ONLY:
        vma_usage = VMA_MEMORY_USAGE_GPU_ONLY;
        break;
    case EBufferAccess::CPU_TO_GPU:
        vma_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
    case EBufferAccess::GPU_TO_CPU:
        vma_usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        break;
    default:;
    }

    const VkBufferCreateInfo buffer_create_info = {
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = NULL,
        .size                  = get_size(),
        .usage                 = vk_usage,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
    };

    const VmaAllocationCreateInfo allocInfo = {
        .flags          = NULL,
        .usage          = vma_usage,
        .requiredFlags  = NULL,
        .preferredFlags = NULL,
        .memoryTypeBits = 0,
        .pool           = VK_NULL_HANDLE,
        .pUserData      = nullptr,
    };

    VK_CHECK(vmaCreateBuffer(vulkan::get_vma_allocator(), &buffer_create_info, &allocInfo, &buffer, &memory, nullptr), "failed to create buffer");
}

void* Buffer_VK::get_ptr()
{
    VmaAllocationInfo allocation_infos;
    vmaGetAllocationInfo(get_vma_allocator(), memory, &allocation_infos);

    void* dst_ptr;
    VK_CHECK(vkMapMemory(vulkan::get_device(), allocation_infos.deviceMemory, 0, get_size(), NULL, (void**)(&dst_ptr)), "failed to map memory");
    return dst_ptr;
}

void Buffer_VK::submit_data()
{
    VmaAllocationInfo allocation_infos;
    vmaGetAllocationInfo(get_vma_allocator(), memory, &allocation_infos);

    vkUnmapMemory(get_device(), allocation_infos.deviceMemory);
}
Buffer_VK::~Buffer_VK()
{
    vkDeviceWaitIdle(get_device());
    vmaDestroyBuffer(get_vma_allocator(), buffer, memory);
}

void Buffer_VK::set_data(void* data, size_t data_length, size_t offset)
{
    if (buffer_access == EBufferAccess::GPU_ONLY)
    {
        LOG_ERROR("cannot set data from CPU on a GPU only buffer");
        return;
    }

    if (data_length == 0)
    {
        LOG_ERROR("trying to set buffer data with empty data");
        return;
    }

    if (data_length + offset > get_size())
    {
        LOG_ERROR("trying to set buffer data of size %lu, with data of size %lu and offset %lu", get_size(), data_length, offset);
        return;
    }

#if GFX_USE_VULKAN
    VmaAllocationInfo allocation_infos;
    vmaGetAllocationInfo(vulkan::get_vma_allocator(), memory, &allocation_infos);

    void* dst_ptr;
    VK_CHECK(vkMapMemory(vulkan::get_device(), allocation_infos.deviceMemory, offset, data_length, NULL, (void**)(&dst_ptr)), "failed to map memory");
    memcpy(dst_ptr, data, data_length);
    vkUnmapMemory(vulkan::get_device(), allocation_infos.deviceMemory);
#else
    (void)data;
#endif
}

void Buffer_VK::bind_buffer(VkCommandBuffer command_buffer)
{
    if (usage == EBufferUsage::INDEX_DATA)
    {
        vkCmdBindIndexBuffer(command_buffer, buffer, 0, VK_INDEX_TYPE_UINT32);
    }
    else if (usage == EBufferUsage::VERTEX_DATA)
    {
        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &buffer, offsets);
    }
}
} // namespace gfx::vulkan