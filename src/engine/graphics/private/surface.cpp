#include "gfx/surface.h"

#include <vulkan/instance.h>
#include <vulkan/win32/win32_surface_builder.h>

#if GFX_USE_VULKAN
#include "vulkan/vk_surface.h"
#endif

namespace gfx
{
Surface* Surface::create_surface(application::window::Window* container)
{
#if GFX_USE_VULKAN
    return new vulkan::Surface_VK(container);
#endif
    return nullptr;
}
} // namespace gfx
