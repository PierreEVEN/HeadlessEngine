#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace gfx::vulkan
{
    namespace allocator
    {
    void create();
    void destroy();
    }


const VkAllocationCallbacks* get_allocator();
} // namespace gfx::vulkan