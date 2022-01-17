#pragma once

#include "vulkan/vk_errors.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_physical_device.h"
#include "vulkan/vk_instance.h"

#include <vulkan/vulkan.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <cpputils/logger.hpp>

namespace gfx::vulkan
{
static VmaAllocator vulkan_memory_allocator = VK_NULL_HANDLE;

namespace allocator
{
void create()
{
    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = GET_VK_PHYSICAL_DEVICE(),
        .device         = get_device(),
        .instance       = get_instance(),
    };
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vulkan_memory_allocator), "failed to create vma allocator");
}

void destroy()
{
    vmaDestroyAllocator(vulkan_memory_allocator);
}

} // namespace allocator

const VkAllocationCallbacks* get_allocator()
{
    return nullptr;
}

const VmaAllocator& get_vma_allocator()
{
    if (vulkan_memory_allocator == VK_NULL_HANDLE)
        LOG_FATAL("vulkan memory allocator should have been created first");
    
    return vulkan_memory_allocator;
}
} // namespace gfx::vulkan