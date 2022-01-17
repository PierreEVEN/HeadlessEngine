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

const VmaAllocator& get_vma_allocator();


} // namespace gfx::vulkan