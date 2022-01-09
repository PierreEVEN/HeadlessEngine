#pragma once

#include <array>

#include <vulkan/vulkan.hpp>

namespace gfx::config
{
#ifdef ENABLE_VALIDATION_LAYER
constexpr std::initializer_list<const char*> validation_layers   = {"VK_LAYER_KHRONOS_validation"};
constexpr std::initializer_list<const char*> required_extensions = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#if APP_USE_WIN32
    "VK_KHR_win32_surface",
#endif
};
#else
constexpr std::initializer_list<const char*> validation_layers   = {};
constexpr std::initializer_list<const char*> required_extensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if APP_USE_WIN32
    "VK_KHR_win32_surface",
#endif
};
#endif

#ifdef ENABLE_VALIDATION_LAYER
constexpr std::initializer_list<const char*> device_extensions = {"VK_KHR_swapchain", VK_EXT_DEBUG_MARKER_EXTENSION_NAME};
#else
constexpr std::initializer_list<const char*> device_extensions = {"VK_KHR_swapchain"};
#endif
} // namespace gfx::config
