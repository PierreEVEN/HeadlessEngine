
#include "gfx/render_pass.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/vk_render_pass.h"
#endif

namespace gfx
{

static std::unordered_map<std::string, std::unique_ptr<RenderPass>> render_passes;



RenderPass::RenderPass(uint32_t framebuffer_width, uint32_t framebuffer_height, const RenderPassConfig& frame_graph_config)
    : width(framebuffer_width), height(framebuffer_height), config(frame_graph_config)
{
}

RenderPass* get_render_pass(const std::string& render_pass_name)
{
    const auto& it = render_passes.find(render_pass_name);
    if (it != render_passes.end())
        return it->second.get();
    return nullptr;
}

std::shared_ptr<RenderPass> RenderPass::create(uint32_t framebuffer_width, uint32_t framebuffer_height, const RenderPassConfig& frame_graph_config)
{
#if GFX_USE_VULKAN
    return std::make_shared<vulkan::RenderPass_VK>(framebuffer_width, framebuffer_height, frame_graph_config);
#endif
    return nullptr;
}

void RenderPass::add_child(const std::shared_ptr<RenderPass>& render_pass)
{
    render_pass->parents.emplace_back(this);
    children.emplace_back(render_pass);
}

void RenderPass::draw_pass(CommandBuffer* command_buffer)
{
    for (const auto& child : children)
    {
        child->draw_pass(command_buffer);
    }

    begin(command_buffer);

    // command_buffer.draw_mesh(nullptr, nullptr, 0);

    end(command_buffer);
}

void RenderPass::set_framebuffer_images(const std::vector<std::shared_ptr<Texture>>& images)
{
    resource_render_target = images;
    init();
}

void RenderPass::generate_framebuffer_images()
{
    for (const auto& attachment : config.color_attachments)
    {
        if (Texture::is_depth_format(attachment.image_format))
            LOG_FATAL("Cannot use depth format on color attachments");
        resource_render_target.emplace_back(Texture::create(width, height, 1,
                                                            TextureParameter{
                                                                .format                 = attachment.image_format,
                                                                .transfer_capabilities  = ETextureTransferCapabilities::None,
                                                                .gpu_write_capabilities = ETextureGPUWriteCapabilities::Enabled,
                                                                .mip_level              = 1,
                                                            }));
    }
    if (config.depth_attachment)
    {
        if (!Texture::is_depth_format(config.depth_attachment->image_format))
            LOG_FATAL("Depth attachment require a depth format");
        resource_render_target.emplace_back(Texture::create(width, height, 1,
                                                            TextureParameter{
                                                                .format                 = config.depth_attachment->image_format,
                                                                .transfer_capabilities  = ETextureTransferCapabilities::None,
                                                                .gpu_write_capabilities = ETextureGPUWriteCapabilities::Enabled,
                                                                .mip_level = 1,
                                                            }));
    }
    init();
}
} // namespace gfx
