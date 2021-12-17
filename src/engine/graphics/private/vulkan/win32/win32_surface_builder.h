#pragma once

#include "application/window.h"

#include <vulkan/vulkan.h>

namespace gfx::win32
{
VkSurfaceKHR create_surface(VkInstance instance, application::window::Window* owning_window);

} // namespace gfx::win32