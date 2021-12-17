#pragma once

#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{

VkDebugUtilsMessengerCreateInfoEXT get_validation_layer_create_infos();

}