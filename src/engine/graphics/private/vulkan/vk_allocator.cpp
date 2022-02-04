#pragma once

#include "vulkan/vk_errors.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_physical_device.h"
#include "vulkan/vk_instance.h"

#include <vulkan/vulkan.h>

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{
namespace allocator
{
void create()
{
}

void destroy()
{
}

} // namespace allocator

const VkAllocationCallbacks* get_allocator()
{
    return nullptr;
}
} // namespace gfx::vulkan