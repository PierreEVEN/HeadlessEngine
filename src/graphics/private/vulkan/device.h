#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace gfx::vulkan
{

namespace device
{
void create();
void destroy();

} // namespace device
VkDevice get_device();

} // namespace gfx::vulkan