#include "gfx/render_pass_instance.h"

#include "gfx/materials/master_material.h"
#include "gfx/materials/material_instance.h"
#include "gfx/mesh.h"

#if GFX_USE_VULKAN
#include "vulkan/vk_render_pass_instance.h"
#endif

namespace gfx
{

std::shared_ptr<RenderPassInstance> RenderPassInstance::create(uint32_t width, uint32_t height, RenderPass* base, const std::optional<std::vector<std::shared_ptr<Texture>>>& images)
{
#if GFX_USE_VULKAN
    return std::make_shared<vulkan::RenderPassInstance_VK>(width, height, base, images);
#else
    return nullptr;
#endif
}

RenderPassInstance::RenderPassInstance(uint32_t width, uint32_t height, RenderPass* base, const std::optional<std::vector<std::shared_ptr<Texture>>>& images)
    : render_pass_base(base), framebuffer_width(width), framebuffer_height(height)
{
    if (!render_pass_base)
        LOG_FATAL("render pass base is null");

    if (images)
    {
        framebuffers_images = images.value();
    }
    else
    {
        for (const auto& attachment : render_pass_base->get_config().color_attachments)
        {
            if (Texture::is_depth_format(attachment.image_format))
                LOG_FATAL("Cannot use depth format on color attachments");
            framebuffers_images.emplace_back(Texture::create(width, height, 1,
                                                             TextureParameter{
                                                                 .format                 = attachment.image_format,
                                                                 .transfer_capabilities  = ETextureTransferCapabilities::None,
                                                                 .gpu_write_capabilities = ETextureGPUWriteCapabilities::Enabled,
                                                                 .mip_level              = 1,
                                                             }));
        }
        if (render_pass_base->get_config().depth_attachment)
        {
            if (!Texture::is_depth_format(render_pass_base->get_config().depth_attachment->image_format))
                LOG_FATAL("Depth attachment require a depth format");
            framebuffers_images.emplace_back(Texture::create(width, height, 1,
                                                             TextureParameter{
                                                                 .format                 = render_pass_base->get_config().depth_attachment->image_format,
                                                                 .transfer_capabilities  = ETextureTransferCapabilities::None,
                                                                 .gpu_write_capabilities = ETextureGPUWriteCapabilities::Enabled,
                                                                 .mip_level              = 1,
                                                             }));
        }
    }
}

void RenderPassInstance::build_framegraph()
{
    if (is_generated)
        return;
    is_generated = true;

    for (const auto& child : children)
        child->build_framegraph();

    for (const auto& child : children)
    {
        for (auto& image : child->framebuffers_images)
            available_images.emplace_back(image.get());
        for (const auto& image : child->available_images)
            available_images.emplace_back(image);
    }
}

void RenderPassInstance::link_dependency(const std::shared_ptr<RenderPassInstance>& render_pass)
{
    render_pass->parents.emplace_back(this);
    children.emplace_back(render_pass);
}

void RenderPassInstance::draw_pass(CommandBuffer* command_buffer)
{
    for (const auto& child : children)
    {
        child->draw_pass(command_buffer);
    }

    command_buffer->render_pass = &get_base()->get_id();
    begin(command_buffer);

    if (draw_interface)
        draw_interface->draw(command_buffer);

    end(command_buffer);
}

} // namespace gfx