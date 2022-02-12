#include "vk_resource_buffer.h"

#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"

namespace gfx::vulkan
{

BufferResource_VK::BufferResource_VK(const std::string& name, const CI_Buffer& create_infos)
{
    VkBufferUsageFlags vk_usage  = 0;
    VmaMemoryUsage     vma_usage = VMA_MEMORY_USAGE_UNKNOWN;

    switch (create_infos.usage)
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

    if (create_infos.type != EBufferType::IMMUTABLE)
    {
        vk_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        vk_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    switch (create_infos.access)
    {
    default:
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
    }

    const VkBufferCreateInfo buffer_create_info = {
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = NULL,
        .size                  = create_infos.count * create_infos.stride,
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
    VK_CHECK(vmaCreateBuffer(get_vma_allocator(), &buffer_create_info, &allocInfo, &buffer, &memory, nullptr), "failed to create get shreked");
    debug_set_object_name(stringutils::format("buffer:%s", name.c_str()), buffer);

    
    VmaAllocationInfo allocation_infos;
    vmaGetAllocationInfo(get_vma_allocator(), memory, &allocation_infos);
    debug_set_object_name(stringutils::format("memory:%s", name.c_str()), allocation_infos.deviceMemory);

    descriptor_infos = VkDescriptorBufferInfo{
        .buffer = buffer,
        .offset = 0,
        .range  = create_infos.count * create_infos.stride,
    };
}

BufferResource_VK::~BufferResource_VK()
{
    vmaDestroyBuffer(get_vma_allocator(), buffer, memory);
}

void* BufferResource_VK::map_buffer()
{
    void* dst_ptr;
    VK_CHECK(vmaMapMemory(get_vma_allocator(), memory, &dst_ptr), "failed to map memory");
    return dst_ptr;
}

void BufferResource_VK::unmap_buffer()
{
    vmaUnmapMemory(get_vma_allocator(), memory);
}

}
