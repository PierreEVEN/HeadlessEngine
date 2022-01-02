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

    const std::vector resolve_attachment = {RenderPassConfig::Attachment{
        .attachment_name = "color",
        .image_format    = vulkan::VkTexture::engine_texture_format_from_vk(surface->get_surface_format().format),
    }};

    surface->main_render_pass = std::make_unique<vulkan::RenderPass_VK>(container->width(), container->height(),
                                                                        RenderPassConfig{
                                                                            .pass_name         = "resolve_pass",
                                                                            .color_attachments = resolve_attachment,
                                                                        });

    dynamic_cast<vulkan::RenderPass_VK*>(surface->main_render_pass.get())->is_present_pass = true;
    surface->main_render_pass->set_framebuffer_images({surface->get_surface_render_texture()});
    return surface;
#else
    return nullptr;
#endif
}
} // namespace gfx
