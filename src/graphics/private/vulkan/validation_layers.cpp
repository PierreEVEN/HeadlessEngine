
#include "validation_layers.h"

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{
VKAPI_ATTR VkBool32 VKAPI_CALL validation_layer_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                               const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        LOG_INFO("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        LOG_VALIDATE("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LOG_WARNING("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        LOG_FATAL("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
    }
    else
    {
        LOG_ERROR("VULKAN VALIDATION LAYER - UNKOWN VERBOSITY : %s", callback_data->pMessage);
    }

    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT get_validation_layer_create_infos()
{
    return {
        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext           = nullptr,
        .flags           = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = validation_layer_debug_callback,
        .pUserData       = nullptr,
    };
}
} // namespace gfx::vulkan
