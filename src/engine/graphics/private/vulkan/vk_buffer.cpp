

#include "vk_buffer.h"

#include "types/magic_enum.h"
#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_errors.h"
#include "vk_one_time_command_buffer.h"

#include <string>
#include <vulkan/vk_allocator.h>

#include "glm/glm.hpp"

namespace gfx::vulkan
{
Buffer_VK::Buffer_VK(const std::string& buffer_name, uint32_t buffer_stride, uint32_t in_element_count, EBufferUsage buffer_usage, EBufferAccess in_buffer_access, EBufferType buffer_type)
    : Buffer(buffer_name, buffer_stride, in_element_count, buffer_usage, in_buffer_access, buffer_type), allocated_count(element_count),
      frame_data(buffer_type == EBufferType::STATIC ? SwapchainImageResource<FrameData>::make_static() : SwapchainImageResource<FrameData>::make_dynamic())
{
    if (type == EBufferType::DYNAMIC)
        dynamic_data = new uint8_t[allocated_count * stride()];

    for (auto& data : frame_data)
        create_or_recreate_buffer(data);
}

void Buffer_VK::create_or_recreate_buffer(FrameData& current_buffer)
{

    VkBuffer      previous_buffer = current_buffer.buffer;
    VmaAllocation previous_memory = current_buffer.memory;

    VkBufferUsageFlags vk_usage  = 0;
    VmaMemoryUsage     vma_usage = VMA_MEMORY_USAGE_UNKNOWN;

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

    if (type != EBufferType::IMMUTABLE)
    {
        vk_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        vk_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

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
        .size                  = allocated_count * stride(),
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

    VK_CHECK(vmaCreateBuffer(vulkan::get_vma_allocator(), &buffer_create_info, &allocInfo, &current_buffer.buffer, &current_buffer.memory, nullptr), "failed to create buffer");

    current_buffer.buffer_infos.buffer = current_buffer.buffer;
    current_buffer.buffer_infos.offset = 0;
    current_buffer.buffer_infos.range  = size();

    if (previous_buffer != VK_NULL_HANDLE)
    {
        // Copy previous data
        if (type == EBufferType::STATIC && current_buffer.previous_allocated_count != 0) //@TODO : delay copy to avoid freeze
        {
            OneTimeCommandBuffer copy_cmd;
            const VkBufferCopy   copy_region{
                .srcOffset = 0,
                .dstOffset = 0,
                .size      = current_buffer.previous_allocated_count * stride(),
            };
            vkCmdCopyBuffer(*copy_cmd, previous_buffer, current_buffer.buffer, 1, &copy_region);
        }
        // Destroy previous buffer
        vmaDestroyBuffer(get_vma_allocator(), previous_buffer, previous_memory);
    }
}

void Buffer_VK::resize(uint32_t in_element_count)
{
    if (type == EBufferType::IMMUTABLE)
    {
        LOG_ERROR("An immutable buffer is not resizable");
        return;
    }

    element_count                  = in_element_count;
    frame_data->buffer_infos.range = size();

    if (type == EBufferType::STATIC)
    {
        allocated_count = in_element_count;
    }
    else
    {
        if (in_element_count > allocated_count || in_element_count < allocated_count / 4)
        {
            allocated_count = in_element_count * 2;
            if (type == EBufferType::DYNAMIC)
            {
                delete[] dynamic_data;
                dynamic_data = new uint8_t[allocated_count * stride()];
            }
        }
    }

    resize_current();
}

void* Buffer_VK::acquire_data_ptr()
{
    if (type == EBufferType::DYNAMIC)
        return dynamic_data;

    if (type == EBufferType::STATIC)
        vkDeviceWaitIdle(get_device());

    void* dst_ptr;
    VK_CHECK(vmaMapMemory(vulkan::get_vma_allocator(), frame_data->memory, &dst_ptr), "failed to map memory");
    return dst_ptr;
}

void Buffer_VK::submit_data()
{
    if (type == EBufferType::DYNAMIC)
        for (auto& data : frame_data)
            data.dirty = true;
    else
        vmaUnmapMemory(get_vma_allocator(), frame_data->memory);
}

void Buffer_VK::resize_current()
{
    if (frame_data->allocated_count != allocated_count)
    {
        frame_data->allocated_count = allocated_count;
        if (vmaResizeAllocation(get_vma_allocator(), frame_data->memory, frame_data->allocated_count * stride()) != VK_SUCCESS)
        {
            create_or_recreate_buffer(*frame_data);
        }
        frame_data->buffer_infos.range       = size();
        frame_data->previous_allocated_count = allocated_count;
    }
}

Buffer_VK::~Buffer_VK()
{
    delete[] dynamic_data;
    vkDeviceWaitIdle(get_device());
    for (const auto& data : frame_data)
        vmaDestroyBuffer(get_vma_allocator(), data.buffer, data.memory);
}

void Buffer_VK::bind_buffer(VkCommandBuffer command_buffer)
{
    if (type == EBufferType::DYNAMIC && frame_data->dirty)
    {
        resize_current();
        void* dst_ptr;
        VK_CHECK(vmaMapMemory(vulkan::get_vma_allocator(), frame_data->memory, &dst_ptr), "failed to map memory");
        memcpy(dst_ptr, dynamic_data, size());
        vmaUnmapMemory(get_vma_allocator(), frame_data->memory);
        frame_data->dirty = false;
    }

    if (usage == EBufferUsage::INDEX_DATA)
    {
        vkCmdBindIndexBuffer(command_buffer, frame_data->buffer, 0, stride() == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    }
    else if (usage == EBufferUsage::VERTEX_DATA)
    {
        constexpr VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &frame_data->buffer, offsets);
    }
    else
    {
        LOG_WARNING("unhandled buffer type");
    }
}
} // namespace gfx::vulkan