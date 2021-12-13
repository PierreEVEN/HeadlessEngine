#pragma once

#define GLFW_INCLUDE_VULKAN
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

namespace gfx::vulkan
{
    namespace allocator
    {
    void create();
    void destroy();

    }


const VkAllocationCallbacks* get_allocator();

const VmaAllocator& get_vma_allocator();


} // namespace gfx::vulkan