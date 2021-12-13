#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace gfx::vulkan
{

VkDebugUtilsMessengerCreateInfoEXT get_validation_layer_create_infos();

}