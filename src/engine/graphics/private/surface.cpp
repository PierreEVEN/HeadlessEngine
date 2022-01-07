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
    
    if (!RenderPassID::exists("resolve_pass"))
    {
        RenderPass::declare_internal(
            RenderPass::Config{
                .pass_name         = "resolve_pass",
                .color_attachments = {RenderPass::Config::Attachment{
                    .attachment_name = "color",
                    .image_format    = vulkan::Texture_VK::engine_texture_format_from_vk(surface->get_surface_format().format),
                }},
            },
            true);
    }

    surface->main_render_pass = RenderPassInstance::create(container->width(), container->height(), RenderPassID::get("resolve_pass"), std::vector{surface->get_surface_render_texture()});
    return surface;
#else
    return nullptr;
#endif
}

void Surface::build_framegraph()
{
    main_render_pass->build_framegraph();
}
} // namespace gfx
