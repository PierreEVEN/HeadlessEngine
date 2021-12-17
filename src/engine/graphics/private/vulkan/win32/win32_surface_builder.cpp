
#define VK_USE_PLATFORM_WIN32_KHR
#include "application/application.h"
#include "vulkan/allocator.h"

#include <vulkan/vulkan.hpp>

#include "win32_surface_builder.h"

namespace gfx::win32
{

VkSurfaceKHR create_surface(VkInstance instance, application::window::Window* owning_window)
{

    VkWin32SurfaceCreateInfoKHR create_infos = {
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = nullptr,
        .flags     = NULL,
        .hinstance = reinterpret_cast<HINSTANCE>(application::get()->get_platform_app_handle()),
        .hwnd      = reinterpret_cast<HWND>(owning_window->get_platform_window_handle()),
    };

    VkSurfaceKHR surface;
    vkCreateWin32SurfaceKHR(instance, &create_infos, vulkan::get_allocator(), &surface);
    return surface;
}
} // namespace gfx::win32