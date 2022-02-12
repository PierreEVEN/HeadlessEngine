

#include "vk_buffer.h"

#include "gfx/resource/gpu_resource.h"
#include "types/magic_enum.h"
#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_errors.h"
#include "vk_one_time_command_buffer.h"

#include <string>

#include "glm/glm.hpp"
#include <vulkan/vk_helper.h>

namespace gfx::vulkan
{
Buffer_VK::Buffer_VK(const std::string& in_buffer_name, uint32_t buffer_stride, uint32_t in_element_count, EBufferUsage buffer_usage, EBufferAccess in_buffer_access, EBufferType buffer_type)
    : Buffer(in_buffer_name, buffer_stride, in_element_count, buffer_usage, in_buffer_access, buffer_type), allocated_count(element_count),
      frame_data(buffer_type == EBufferType::STATIC || buffer_type == EBufferType::IMMUTABLE ? SwapchainImageResource<FrameData>::make_static() : SwapchainImageResource<FrameData>::make_dynamic())
{
    if (type == EBufferType::DYNAMIC)
        dynamic_data = new uint8_t[allocated_count * stride()];

    for (auto& data : frame_data)
        create_or_recreate_buffer(data);
}

void Buffer_VK::create_or_recreate_buffer(FrameData& current_buffer)
{
    TGpuHandle<BufferResource_VK> previous_buffer = current_buffer.buffer;

    current_buffer.buffer = TGpuHandle<BufferResource_VK>("buffer_" + buffer_name, BufferResource_VK::CI_Buffer{
                                                                                       .stride = stride(),
                                                                                       .count  = allocated_count,
                                                                                       .usage  = usage,
                                                                                       .access = buffer_access,
                                                                                       .type   = type,
                                                                                   });

    if (previous_buffer)
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
            vkCmdCopyBuffer(*copy_cmd, previous_buffer->buffer, current_buffer.buffer->buffer, 1, &copy_region);
        }
    }

    if (previous_buffer)
        previous_buffer.destroy();
}

void Buffer_VK::resize(uint32_t in_element_count)
{
    if (type == EBufferType::IMMUTABLE)
    {
        LOG_ERROR("An immutable get is not resizable");
        return;
    }

    element_count                              = in_element_count;
    frame_data->buffer->descriptor_infos.range = size();

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

    return frame_data->buffer->map_buffer();
}

void Buffer_VK::submit_data()
{
    if (type == EBufferType::DYNAMIC)
        for (auto& data : frame_data)
            data.dirty = true;
    else
        frame_data->buffer->unmap_buffer();
}

void Buffer_VK::resize_current()
{
    if (frame_data->allocated_count != allocated_count)
    {
        frame_data->allocated_count = allocated_count;
        if (vmaResizeAllocation(get_vma_allocator(), frame_data->buffer->get_allocation(), frame_data->allocated_count * stride()) == VK_SUCCESS)
            frame_data->buffer->descriptor_infos.range = size();
        else
            create_or_recreate_buffer(*frame_data);
        frame_data->previous_allocated_count = allocated_count;
    }
}

Buffer_VK::~Buffer_VK()
{
    delete[] dynamic_data;
}

void Buffer_VK::bind_buffer(VkCommandBuffer command_buffer)
{
    if (type == EBufferType::DYNAMIC && frame_data->dirty)
    {
        resize_current();
        memcpy(frame_data->buffer->map_buffer(), dynamic_data, size());
        frame_data->buffer->unmap_buffer();
        frame_data->dirty = false;
    }

    if (usage == EBufferUsage::INDEX_DATA)
    {
        vkCmdBindIndexBuffer(command_buffer, frame_data->buffer->buffer, 0, stride() == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    }
    else if (usage == EBufferUsage::VERTEX_DATA)
    {
        constexpr VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &frame_data->buffer->buffer, offsets);
    }
    else
    {
        LOG_WARNING("This get can't be bound to a command get");
    }
}
} // namespace gfx::vulkan