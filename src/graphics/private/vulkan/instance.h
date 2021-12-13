#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace gfx::vulkan
{
namespace instance
{
void create();
void destroy();
}

const VkInstance& get_instance();

} // namespace gfx::vulkan