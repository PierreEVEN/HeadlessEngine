
#include "application/application.h"
#include "assertion.h"
#include "config.h"
#include "validation_layers.h"
#include "vulkan/allocator.h"

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{

static VkInstance vulkan_instance = VK_NULL_HANDLE;

namespace instance
{

void create()
{
    /* Set application information */

    const char* application_name = application::get_full_name().c_str();
    const char* engine_name      = application::get_engine_full_name().c_str();

    VkApplicationInfo vkAppInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = nullptr,
        .pApplicationName   = application_name,
        .applicationVersion = VK_MAKE_VERSION(application::get_version_major(), application::get_version_minor(), application::get_version_patch()),
        .pEngineName        = engine_name,
        .engineVersion      = VK_MAKE_VERSION(application::get_engine_version_major(), application::get_engine_version_minor(), application::get_engine_version_patch()),
        .apiVersion         = VK_API_VERSION_1_2,
    };

    /* Initialize vulkan instance */
    VkInstanceCreateInfo vkInstanceCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &vkAppInfo,
        .enabledExtensionCount   = static_cast<uint32_t>(config::required_extensions.size()),
        .ppEnabledExtensionNames = config::required_extensions.begin(),
    };
    
#ifdef ENABLE_VALIDATION_LAYER
    auto validation_layer_create_infos       = get_validation_layer_create_infos();
    vkInstanceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(config::validation_layers.size());
    vkInstanceCreateInfo.ppEnabledLayerNames = config::validation_layers.begin();
    vkInstanceCreateInfo.pNext               = &validation_layer_create_infos;
    LOG_INFO("[ Core] Linked validation layers");
#else
    vkInstanceCreateInfo.enabledLayerCount = 0;
    vkInstanceCreateInfo.pNext             = nullptr;
#endif

    VK_CHECK(vkCreateInstance(&vkInstanceCreateInfo, get_allocator(), &vulkan_instance), "Failed to create vulkan instance");
}

void destroy()
{
    vkDestroyInstance(vulkan_instance, get_allocator());
}
} // namespace instance

const VkInstance& get_instance()
{
    if (vulkan_instance == VK_NULL_HANDLE)
        LOG_FATAL("vulkan instance should have been created first");

    return vulkan_instance;
}
} // namespace gfx::vulkan