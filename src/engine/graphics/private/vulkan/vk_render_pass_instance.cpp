
#include "vk_render_pass_instance.h"

#include "vulkan/vk_errors.h"
#include "vulkan/vk_render_pass.h"
#include "vulkan/vk_texture.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_command_buffer.h"
#include "vulkan/vk_helper.h"

namespace gfx::vulkan
{
RenderPassInstance_VK::RenderPassInstance_VK(uint32_t width, uint32_t height, RenderPass* base, const std::optional<std::vector<std::shared_ptr<Texture>>>& images) : RenderPassInstance(width, height, base, images)
{
    resize(width, height);
}

RenderPassInstance_VK::~RenderPassInstance_VK()
{
    for (const auto& framebuffer : framebuffers)
        vkDestroyFramebuffer(get_device(), framebuffer, get_allocator());
}

void gfx::vulkan::RenderPassInstance_VK::begin(CommandBuffer* command_buffer)
{
#if GFX_USE_VULKAN
    RenderPass_VK* base = static_cast<RenderPass_VK*>(get_base());

    command_buffer->render_pass  = base->get_config().pass_name;
    CommandBuffer_VK* cmd_buffer = dynamic_cast<CommandBuffer_VK*>(command_buffer);

    VkCommandBuffer& cmd = **cmd_buffer;

    debug_add_marker("draw render pass [" + base->get_config().pass_name + "]", cmd, {0.5f, 1.0f, 0.5f, 1.0f});

    const VkRenderPassBeginInfo begin_infos = {
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass  = base->get(),
        .framebuffer = *framebuffers,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = {get_width(), get_height()},
            },
        //.clearValueCount = static_cast<uint32_t>(clear_values.size()),
        //.pClearValues    = clear_values.data(),
    };
    vkCmdBeginRenderPass(cmd, &begin_infos, VK_SUBPASS_CONTENTS_INLINE);

    const VkViewport viewport{
        .x        = 0,
        .y        = static_cast<float>(get_height()), // Flip viewport vertically to avoid textures to be displayed upside down
        .width    = static_cast<float>(get_width()),
        .height   = -static_cast<float>(get_height()),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const VkRect2D scissor{
        .offset = VkOffset2D{0, 0},
        .extent = VkExtent2D{get_width(), get_height()},
    };

    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
#endif
}

void RenderPassInstance_VK::end(CommandBuffer* command_buffer)
{
#if GFX_USE_VULKAN
    CommandBuffer_VK* cmd_buffer = dynamic_cast<CommandBuffer_VK*>(command_buffer);
    VkCommandBuffer&  cmd        = **cmd_buffer;
    vkCmdEndRenderPass(cmd);

    debug_end_marker(cmd);
#endif
}

void RenderPassInstance_VK::resize(uint32_t width, uint32_t height)
{
    RenderPass_VK* base = static_cast<RenderPass_VK*>(get_base());

    framebuffer_width  = width;
    framebuffer_height = height;
    for (uint8_t i = 0; i < framebuffers.get_max_instance_count(); ++i)
    {
        std::vector<VkImageView> attachments(0);
        for (const auto& image : framebuffers_images)
        {
            const auto texture = dynamic_cast<Texture_VK*>(image.get());
            attachments.emplace_back(texture->get_view()[i]);
        }

        const VkFramebufferCreateInfo framebuffer_infos{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = base->get(),
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments    = attachments.data(),
            .width           = framebuffer_width,
            .height          = framebuffer_height,
            .layers          = 1,
        };

        VK_CHECK(vkCreateFramebuffer(get_device(), &framebuffer_infos, get_allocator(), &framebuffers[i]), "Failed to create framebuffers");
        debug_set_object_name(stringutils::format("render pass %s : framebuffers #%d", get_base()->get_config().pass_name.c_str(), i), framebuffers[i]);
    }
}
} // namespace gfx::vulkan