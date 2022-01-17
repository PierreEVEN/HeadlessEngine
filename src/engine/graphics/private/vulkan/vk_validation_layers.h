#pragma once

#include <vulkan/vulkan.h>

namespace gfx::vulkan
{

VkDebugUtilsMessengerCreateInfoEXT get_validation_layer_create_infos();

}