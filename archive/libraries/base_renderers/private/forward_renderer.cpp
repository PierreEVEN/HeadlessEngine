

#include "forward_renderer.h"

#include "rendering/graphics.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/common.h"

namespace ForwardRenderer
{

RendererConfiguration create_configuration()
{
    return RendererConfiguration({
        RenderPassSettings{
            .pass_name    = "render_scene",
            .sample_count = static_cast<VkSampleCountFlagBits>(vulkan_common::get_msaa_sample_count()),
            .color_attachments =
                std::vector<RenderPassAttachment>{
                    {
                        .image_format = Graphics::get()->get_swapchain_config()->get_surface_format().format,
                        .clear_value  = std::optional<VkClearValue>({.color = {.float32{0.4f, 0.5f, 0.6f, 1.0f}}}),
                    },
                },
            .depth_attachment =
                RenderPassAttachment{
                    .image_format = vulkan_utils::get_depth_format(),
                    .clear_value  = std::optional<VkClearValue>(VkClearValue{.depthStencil = {.depth = 1, .stencil = 0}}),
                },
        },
    });
}
} // namespace ForwardRenderer