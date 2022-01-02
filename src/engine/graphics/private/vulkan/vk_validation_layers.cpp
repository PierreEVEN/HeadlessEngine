
#include "vulkan/vk_validation_layers.h"

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{
VKAPI_ATTR VkBool32 VKAPI_CALL validation_layer_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                               const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] void* user_data)
{
    std::string context      = "FAILED TO PARSE MESSAGE";
    std::string message_id   = "FAILED TO PARSE MESSAGE";
    std::string message_text = "FAILED TO PARSE MESSAGE";

    const std::string message     = callback_data->pMessage;
    const auto        message_ids = stringutils::split(message, {'|'});
    if (message_ids.size() == 3)
    {
        const auto message_id_vect = stringutils::split(message_ids[1], {'='});
        context                    = message_ids[0];
        message_id                 = message_id_vect.size() == 2 ? message_id_vect[1] : "FAILED TO PARSE MESSAGE ID";
        message_text               = message_ids[2];
    }

    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        LOG_DEBUG("VALIDATION MESSAGE : \n\tcontext : %s\n\tmessage id : %s\n\n\t%s", context.c_str(), message_id.c_str(), message_text.c_str());
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        LOG_INFO("VALIDATION INFO : \n\tcontext : %s\n\tmessage id : %s\n\n\t%s", context.c_str(), message_id.c_str(), message_text.c_str());
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LOG_WARNING("VALIDATION WARNING : \n\tcontext : %s\n\tmessage id : %s\n\n\t%s", context.c_str(), message_id.c_str(), message_text.c_str());
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        LOG_FATAL("VALIDATION ERROR : \n\tcontext : %s\n\tmessage id : %s\n\n\t%s", context.c_str(), message_id.c_str(), message_text.c_str());
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
