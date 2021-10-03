

#include "rendering/vulkan/shader_buffer.h"

#include "rendering/gfx_context.h"
#include "rendering/vulkan/common.h"
#include <cpputils/logger.hpp>

ShaderBufferResource::ShaderBufferResource(const VkBufferUsageFlags& in_buffer_usage) : buffer_usage(in_buffer_usage)
{
    descriptor_buffer_info.offset = 0;
}

ShaderBufferResource::~ShaderBufferResource()
{
    vkDestroyBuffer(GfxContext::get()->logical_device, gpu_buffer, vulkan_common::allocation_callback);
    vkFreeMemory(GfxContext::get()->logical_device, buffer_memory, vulkan_common::allocation_callback);
}

void ShaderBufferResource::set_data(void* data, size_t data_size)
{
    if (data_size > descriptor_buffer_info.range)
    {
        resize_buffer(data_size);
    }

    void* memory_data = nullptr;
    VK_ENSURE(vkMapMemory(GfxContext::get()->logical_device, buffer_memory, 0, data_size, 0, &memory_data), "failed to map memory");
    memcpy(memory_data, data, data_size);
    vkUnmapMemory(GfxContext::get()->logical_device, buffer_memory);
}

void ShaderBufferResource::resize_buffer(size_t data_size)
{
    if (gpu_buffer)
        vkDestroyBuffer(GfxContext::get()->logical_device, gpu_buffer, vulkan_common::allocation_callback);
    if (buffer_memory)
        vkFreeMemory(GfxContext::get()->logical_device, buffer_memory, vulkan_common::allocation_callback);

    vulkan_utils::create_buffer(data_size, buffer_usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, gpu_buffer, buffer_memory);

    descriptor_buffer_info.buffer = gpu_buffer;
    descriptor_buffer_info.range  = data_size;
}

VkDescriptorBufferInfo* ShaderBufferResource::get_descriptor_buffer_info()
{
    VK_CHECK(descriptor_buffer_info.buffer, "buffer has not been initialized");
    return &descriptor_buffer_info;
}