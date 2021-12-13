
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace gfx::vulkan
{
namespace hardware
{
void select_hardware();

}

VkPhysicalDevice get_physical_device();

} // namespace gfx::vulkan