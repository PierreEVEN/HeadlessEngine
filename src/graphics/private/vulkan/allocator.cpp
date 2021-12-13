#pragma once

#define GLFW_INCLUDE_VULKAN
#include "assertion.h"
#include "device.h"
#include "hardware.h"
#include "gfx/instance.h"

#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

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
        .physicalDevice = get_physical_device(),
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