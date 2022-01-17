#include "gfx/surface.h"

#if GFX_USE_VULKAN
#include "vulkan/vk_render_pass.h"
#include "vulkan/vk_surface.h"
#include "vulkan/vk_texture.h"
#endif

namespace gfx
{
Surface* Surface::create_surface(application::window::Window* container)
{
#if GFX_USE_VULKAN
    const auto surface = new vulkan::Surface_VK(container);
    return surface;
#else
    static_assert(false, "backend not supported");
#endif
}

void Surface::link_dependency(const std::shared_ptr<RenderPassInstance>& render_pass) const
{
    main_render_pass->link_dependency(render_pass);
}

void Surface::build_framegraph()
{
    main_render_pass->build_framegraph();
}
} // namespace gfx
