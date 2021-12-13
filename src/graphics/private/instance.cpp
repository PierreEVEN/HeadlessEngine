#include "gfx/instance.h"

#include "vulkan/allocator.h"

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{

static VkInstance vulkan_instance = VK_NULL_HANDLE;

namespace instance
{

}

const VkInstance& get_instance()
{
    if (vulkan_instance == VK_NULL_HANDLE)
        LOG_FATAL("vulkan instance should have been created first");

    return vulkan_instance;
}

} // namespace gfx