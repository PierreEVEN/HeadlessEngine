
#include "vk_surface.h"

#include "allocator.h"
#include "application/application.h"
#include "instance.h"

namespace gfx::vulkan
{
Surface_VK::Surface_VK(application::window::Window* container)
{
#if APP_USE_WIN32

    VkWin32SurfaceCreateInfoKHR create_infos = {
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = nullptr,
        .flags     = NULL,
        .hinstance = reinterpret_cast<HINSTANCE>(application::get()->get_platform_app_handle()),
        .hwnd      = reinterpret_cast<HWND>(container->get_platform_window_handle()),
    };

    vkCreateWin32SurfaceKHR(get_instance(), &create_infos, get_allocator(), &surface);
#endif
}

Surface_VK::~Surface_VK()
{
#if APP_USE_WIN32
    vkDestroySurfaceKHR(get_instance(), surface, get_allocator());
#endif
}

} // namespace gfx::vulkan