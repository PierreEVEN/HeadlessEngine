

#include "gfx/buffer.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/allocator.h"
#include "vulkan/assertion.h"
#include <vulkan/vulkan.hpp>
#include "vulkan/device.h"
#include <vk_mem_alloc.h>
#endif

namespace gfx
{

Buffer::Buffer(const std::string& buffer_name, uint32_t size, EBufferUsage usage, EBufferAccess in_buffer_access) : buffer_name(buffer_name)
{
    create_buffer_internal(size, usage, in_buffer_access);
}

Buffer::~Buffer()
{
#if GFX_USE_VULKAN
    vmaDestroyBuffer(vulkan::get_vma_allocator(), reinterpret_cast<VkBuffer>(buffer_handle), reinterpret_cast<VmaAllocation>(buffer_memory));
#endif
}

void Buffer::set_data(void* data, size_t data_length, size_t offset)
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

    if (data_length + offset > buffer_size)
    {
        LOG_ERROR("trying to set buffer data of size %lu, with data of size %lu and offset %lu", buffer_size, data_length, offset);
        return;
    }

#if GFX_USE_VULKAN
    VmaAllocationInfo allocation_infos;
    vmaGetAllocationInfo(vulkan::get_vma_allocator(), reinterpret_cast<VmaAllocation>(buffer_memory), &allocation_infos);

    void* dst_ptr;
    VK_CHECK(vkMapMemory(vulkan::get_device(), allocation_infos.deviceMemory, offset, data_length, NULL, (void**)(&dst_ptr)));
    memcpy(dst_ptr, data, data_length);
    vkUnmapMemory(vulkan::get_device(), allocation_infos.deviceMemory);
#else
    (void)data;
#endif
}

void Buffer::create_buffer_internal(uint32_t in_buffer_size, EBufferUsage buffer_usage, EBufferAccess in_buffer_access)
{
    buffer_access = in_buffer_access;
    buffer_size = in_buffer_size;

    if (buffer_size == 0)
    {
        LOG_WARNING("trying to create an empty buffer (buffer_size > 0)");
        buffer_size = 1;
    }

#if GFX_USE_VULKAN
    VkBufferUsageFlags vk_usage = 0;
    switch (buffer_usage)
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

    VkBufferCreateInfo buffer_create_info = {
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = NULL,
        .size                  = buffer_size,
        .usage                 = vk_usage,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
    };

    VmaAllocationCreateInfo allocInfo = {
        .flags          = NULL,
        .usage          = vma_usage,
        .requiredFlags  = NULL,
        .preferredFlags = NULL,
        .memoryTypeBits = 0,
        .pool           = VK_NULL_HANDLE,
        .pUserData      = nullptr,
    };

    VK_CHECK(vmaCreateBuffer(vulkan::get_vma_allocator(), &buffer_create_info, &allocInfo, reinterpret_cast<VkBuffer*>(&buffer_handle), reinterpret_cast<VmaAllocation*>(&buffer_memory), nullptr));
#else
    (void)buffer_usage;
#endif
}

} // namespace gfx