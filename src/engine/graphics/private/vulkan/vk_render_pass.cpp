
#include "vk_render_pass.h"

#include "vk_helper.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_command_buffer.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_texture.h"
#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{
RenderPass_VK::RenderPass_VK(uint32_t framebuffer_width, uint32_t framebuffer_height, const RenderPassConfig& frame_graph_config) : RenderPass(framebuffer_width, framebuffer_height, frame_graph_config)
{
}

RenderPass_VK::~RenderPass_VK()
{
    for (const auto& framebuffer : framebuffers)
        vkDestroyFramebuffer(get_device(), framebuffer, get_allocator());

    vkDestroyRenderPass(get_device(), render_pass, get_allocator());
}

void RenderPass_VK::begin(CommandBuffer* command_buffer)
{
#if GFX_USE_VULKAN
    command_buffer->render_pass  = config.pass_name;
    CommandBuffer_VK* cmd_buffer = dynamic_cast<CommandBuffer_VK*>(command_buffer);

    VkCommandBuffer& cmd = **cmd_buffer;

    debug_add_marker("draw render pass [" + config.pass_name + "]", cmd, {0.5f, 1.0f, 0.5f, 1.0f});

    const VkRenderPassBeginInfo begin_infos = {
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass  = render_pass,
        .framebuffer = *framebuffers,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = {width, height},
            },
        //.clearValueCount = static_cast<uint32_t>(clear_values.size()),
        //.pClearValues    = clear_values.data(),
    };
    vkCmdBeginRenderPass(cmd, &begin_infos, VK_SUBPASS_CONTENTS_INLINE);
#endif
}

void RenderPass_VK::end(CommandBuffer* command_buffer)
{
#if GFX_USE_VULKAN
    CommandBuffer_VK* cmd_buffer = dynamic_cast<CommandBuffer_VK*>(command_buffer);
    VkCommandBuffer&  cmd        = **cmd_buffer;
    vkCmdEndRenderPass(cmd);

    debug_end_marker(cmd);
#endif
}

void RenderPass_VK::init()
{
    std::vector<VkAttachmentDescription> attachment_descriptions;
    std::vector<VkAttachmentReference>   color_attachment_references;
    std::optional<VkAttachmentReference> depth_attachment_reference;
    std::optional<VkAttachmentReference> color_attachment_resolve_reference;

    // add color color_attachments
    for (const auto& col_attachment : config.color_attachments)
    {
        if (col_attachment.image_format == ETypeFormat::UNDEFINED)
            LOG_FATAL("images buffer format is undefined");

        const uint32_t attachment_index = static_cast<uint32_t>(attachment_descriptions.size());

        attachment_descriptions.emplace_back(VkAttachmentDescription{
            .format         = Texture_VK::vk_texture_format_to_engine(col_attachment.image_format),
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = col_attachment.clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = is_present_pass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });

        color_attachment_references.emplace_back(VkAttachmentReference{
            .attachment = attachment_index,
            .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        });
    }

    // add depth attachment
    if (config.depth_attachment)
    {
        if (config.depth_attachment->image_format == ETypeFormat::UNDEFINED)
            LOG_FATAL("images buffer format is undefined");

        const uint32_t attachment_index = static_cast<uint32_t>(attachment_descriptions.size());

        attachment_descriptions.emplace_back(VkAttachmentDescription{
            .format         = Texture_VK::vk_texture_format_to_engine(config.depth_attachment->image_format),
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = config.depth_attachment->clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        });

        depth_attachment_reference = VkAttachmentReference{
            .attachment = attachment_index,
            .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
    }

    const VkSubpassDescription subpass = {
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount    = 0,       // Input color_attachments can be used to sample from contents of a previous subpass
        .pInputAttachments       = nullptr, // (Input color_attachments not used here)
        .colorAttachmentCount    = static_cast<uint32_t>(color_attachment_references.size()),
        .pColorAttachments       = color_attachment_references.data(),
        .pResolveAttachments     = color_attachment_resolve_reference ? &color_attachment_resolve_reference.value() : nullptr, // resolve mean the target attachment for msaa
        .pDepthStencilAttachment = depth_attachment_reference ? &depth_attachment_reference.value() : nullptr,
        .preserveAttachmentCount = 0,       // Preserved color_attachments can be used to loop (and preserve) color_attachments through subpasses
        .pPreserveAttachments    = nullptr, // (Preserve color_attachments not used by this example)
    };

    const std::array dependencies{VkSubpassDependency{
                                      .srcSubpass      = VK_SUBPASS_EXTERNAL,                                                        // Producer of the dependency
                                      .dstSubpass      = 0,                                                                          // Consumer is our single subpass that will wait for the execution depdendency
                                      .srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                       // Match our pWaitDstStageMask when we vkQueueSubmit
                                      .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                              // is a loadOp stage for color color_attachments
                                      .srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT,                                                  // semaphore wait already does memory dependency for us
                                      .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // is a loadOp CLEAR access mask for color color_attachments
                                      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                                  },
                                  VkSubpassDependency{
                                      .srcSubpass      = 0,                                                                          // Producer of the dependency is our single subpass
                                      .dstSubpass      = VK_SUBPASS_EXTERNAL,                                                        // Consumer are all commands outside of the renderpass
                                      .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                              // is a storeOp stage for color color_attachments
                                      .dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                       // Do not block any subsequent work
                                      .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // is a storeOp `STORE` access mask for color color_attachments
                                      .dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
                                      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                                  }};

    VkRenderPassCreateInfo render_pass_infos{
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachment_descriptions.size()),
        .pAttachments    = attachment_descriptions.data(),
        .subpassCount    = 1,
        .pSubpasses      = &subpass,
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies   = dependencies.data(),
    };

    VK_CHECK(vkCreateRenderPass(get_device(), &render_pass_infos, get_allocator(), &render_pass), "Failed to create render pass");

    debug_set_object_name("render pass " + config.pass_name, render_pass);

    for (uint8_t i = 0; i < framebuffers.get_max_instance_count(); ++i)
    {
        std::vector<VkImageView> attachments(0);
        for (const auto& image : resource_render_target)
        {
            const auto texture = dynamic_cast<vulkan::Texture_VK*>(image.get());
            attachments.emplace_back(texture->get_view()[i]);
        }

        const VkFramebufferCreateInfo framebuffer_infos{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = render_pass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments    = attachments.data(),
            .width           = width,
            .height          = height,
            .layers          = 1,
        };

        VK_CHECK(vkCreateFramebuffer(get_device(), &framebuffer_infos, get_allocator(), &framebuffers[i]), "Failed to create framebuffer");
        debug_set_object_name(stringutils::format("render pass %s : framebuffer #%d", config.pass_name.c_str(), i), framebuffers[i]);
    }
}
} // namespace gfx::vulkan
