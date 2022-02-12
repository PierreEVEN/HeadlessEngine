#include "gfx/render_pass_instance.h"

#include "gfx/master_material.h"
#include "gfx/material_instance.h"

#if GFX_USE_VULKAN
#include "vulkan/vk_render_pass_instance.h"
#endif

namespace gfx
{

std::shared_ptr<RenderPassInstance> RenderPassInstance::create(uint32_t width, uint32_t height, const RenderPassID& base, const std::vector<std::shared_ptr<Texture>>& images)
{
#if GFX_USE_VULKAN
    return std::make_shared<vulkan::RenderPassInstance_VK>(width, height, base, images);
#else
    static_assert(false, "backend not supported");
#endif
}

RenderPassInstance::~RenderPassInstance()
{
    delete command_buffer;
}

RenderPassInstance::RenderPassInstance(uint32_t width, uint32_t height, const RenderPassID& base, const std::vector<std::shared_ptr<Texture>>& images) : framebuffer_width(width), framebuffer_height(height)
{
    if (!base)
        LOG_FATAL("there is no render pass named %s", base.name().c_str());

    render_pass_base = RenderPass::find(base);
    if (!render_pass_base)
        LOG_FATAL("base render pass is null");

    if (!images.empty())
    {
        // If framebuffer images are provided, use them
        framebuffers_images = images;
    }
    else
    {
        create_or_recreate_framebuffer_images();
    }

    command_buffer = CommandBuffer::create(stringutils::format("pass=%s:depth=#%d", base.name().c_str()));
}

void RenderPassInstance::create_or_recreate_framebuffer_images()
{
    if (get_width() == 0 || get_height() == 0)
        return;

    framebuffers_images.clear();

    uint32_t attachment_index = 0;
    // Else : generate new framebuffer images
    for (const auto& attachment : render_pass_base->get_config().color_attachments)
    {
        if (Texture::is_depth_format(attachment.image_format))
            LOG_FATAL("Cannot use depth format on color attachments");
        framebuffers_images.emplace_back(Texture::create(stringutils::format("pass=%s:color=#%d", render_pass_base->get_config().pass_name.c_str(), attachment_index++), get_width(), get_height(), 1,
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
        framebuffers_images.emplace_back(Texture::create(stringutils::format("pass=%s:depth=#%d", render_pass_base->get_config().pass_name.c_str(), attachment_index), get_width(), get_height(), 1,
                                                         TextureParameter{
                                                             .format                 = render_pass_base->get_config().depth_attachment->image_format,
                                                             .transfer_capabilities  = ETextureTransferCapabilities::None,
                                                             .gpu_write_capabilities = ETextureGPUWriteCapabilities::Enabled,
                                                             .mip_level              = 1,
                                                         }));
    }
}

void RenderPassInstance::build_framegraph()
{
    // Fetch all the required dependencies

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

void RenderPassInstance::draw_pass()
{
    for (const auto& child : children)
        child->draw_pass();

    // Set current render pass
    command_buffer->render_pass = &render_pass_base->get_id();

    // Record command get
    begin_pass();
    on_draw_pass.execute(command_buffer);

    // Submit command get
    submit();
}

} // namespace gfx