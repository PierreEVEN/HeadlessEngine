#pragma once

#include <array>

namespace gfx::config
{
#ifdef ENABLE_VALIDATION_LAYER
constexpr std::initializer_list<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
constexpr std::initializer_list<const char*> required_extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#else
constexpr std::initializer_list<const char*> validation_layers   = {};
constexpr std::initializer_list<const char*> required_extensions = {};
#endif

constexpr std::initializer_list<const char*> device_extensions = {"VK_KHR_swapchain"};
} // namespace gfx::config