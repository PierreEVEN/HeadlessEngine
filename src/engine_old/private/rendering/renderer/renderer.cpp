

#include "rendering/renderer/renderer.h"

#include "rendering/graphics.h"
#include "rendering/renderer/render_pass.h"
#include "rendering/renderer/swapchain.h"
#include "rendering/vulkan/framebuffer.h"

#define MAGIC_ENUM_RANGE_MAX 2048
#include "magic_enum/magic_enum.h"

static std::string add_attachment_string(const RenderPassAttachment& attachment)
{
    std::string attachment_string = "";

    attachment_string += stringutils::format("\t\t- image format = %s\n", magic_enum::enum_name(attachment.image_format).data());
    attachment_string += stringutils::format("\t\t- clear value = %s\n", attachment.clear_value ? "yes" : "no");

    return attachment_string;
}

Renderer::Renderer(Swapchain* in_swapchain) : swapchain(in_swapchain)
{
}

Renderer::~Renderer()
{
}

std::string RendererConfiguration::to_string() const
{
    std::string configuration_string = "";

    for (size_t i = 0; i < pass_descriptions.size(); ++i)
    {
        const auto& pass = pass_descriptions[i];

        configuration_string += stringutils::format("[ %d] : %s\n", i, pass.pass_name.c_str());
        configuration_string += stringutils::format("\t- sample count = %s\n", magic_enum::enum_name(pass.sample_count).data());
        configuration_string += stringutils::format("\t- use swapchain images = %s\n", pass.b_use_swapchain_image ? "yes" : "no");
        configuration_string += stringutils::format("\t- color attachments ( x%d) :\n", pass.color_attachments.size());
        for (size_t j = 0; j < pass.color_attachments.size(); ++j)
        {
            const auto& col_attachment = pass.color_attachments[j];
            configuration_string += stringutils::format("\t\t[ %d]\n%s", j, add_attachment_string(col_attachment).c_str());
        }
        if (pass.depth_attachment)
        {
            configuration_string += stringutils::format("\t- depth attachments :\n");
            configuration_string += add_attachment_string(pass.depth_attachment.value()).c_str();
        }
    }
    return configuration_string;
}

void Renderer::init_or_resize(VkExtent2D in_render_resolution)
{
    Graphics::get()->wait_device();

    render_resolution = in_render_resolution;

    if (render_passes.empty())
    {
        for (const auto& pass_description : renderer_configuration.get_pass_descriptions())
        {
            render_passes.emplace_back(std::make_unique<RenderPass>(pass_description));
            per_pass_framebuffer.emplace_back(std::make_unique<Framebuffer>(render_passes[render_passes.size() - 1].get(), render_resolution, swapchain));
        }
    }
    else
    {
        for (size_t i = 0; i < per_pass_framebuffer.size(); ++i)
        {
            per_pass_framebuffer[i]->resize_framebuffer(in_render_resolution);
        }
    }
}

void Renderer::render_frame(SwapchainFrame& swapchain_frame)
{
    if (!swapchain_frame.is_valid)
        return;

    for (int i = 0; i < render_passes.size(); ++i)
    {
        auto& render_pass_description = renderer_configuration.get_pass_descriptions()[i];
        auto& render_pass             = render_passes[i];
        auto& framebuffer             = per_pass_framebuffer[i];
        auto  clear_values            = render_pass_description.get_clear_values();

        swapchain_frame.render_pass = render_pass_description.pass_name;

        VkRenderPassBeginInfo render_pass_info{
            .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass  = render_pass->get_render_pass(),
            .framebuffer = framebuffer->get(swapchain_frame.image_index),
            .renderArea =
                {
                    .offset = {0, 0},
                    .extent = render_resolution,
                },
            .clearValueCount = static_cast<uint32_t>(clear_values.size()),
            .pClearValues    = clear_values.data(),
        };

        VkViewport viewport{
            .x        = 0,
            .y        = static_cast<float>(render_resolution.height), // Flip viewport vertically to avoid textures to be displayed upside down
            .width    = static_cast<float>(render_resolution.width),
            .height   = -static_cast<float>(render_resolution.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D scissor{
            .offset = VkOffset2D{0, 0},
            .extent = render_resolution,
        };

        vkCmdBeginRenderPass(swapchain_frame.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport(swapchain_frame.command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(swapchain_frame.command_buffer, 0, 1, &scissor);

        render_pass_description.on_pass_rendering.execute(&swapchain_frame);

        vkCmdEndRenderPass(swapchain_frame.command_buffer);
    }
}


void Renderer::set_render_pass_description(const RendererConfiguration& in_renderer_configuration)
{
    renderer_configuration = in_renderer_configuration;

    if (renderer_configuration.get_pass_descriptions().empty())
        LOG_FATAL("you need to specify at least one pass");

    for (const auto& pass : renderer_configuration.get_pass_descriptions())
        if (pass.pass_name.empty())
            LOG_FATAL("you need to specify a name for each render pass");

    LOG_INFO("[ GFX] Set render pass configuration : \n%s", in_renderer_configuration.to_string().c_str());

    auto& last_render_pass                 = renderer_configuration.get_pass_descriptions()[renderer_configuration.get_pass_descriptions().size() - 1];
    last_render_pass.b_use_swapchain_image = true;
}
