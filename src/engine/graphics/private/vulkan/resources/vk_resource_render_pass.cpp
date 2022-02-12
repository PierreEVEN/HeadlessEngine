#include "vk_resource_render_pass.h"

#include "types/type_format.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_types.h"

#include <cpputils/logger.hpp>
#include <optional>

namespace gfx::vulkan
{

RenderPassResource_VK::RenderPassResource_VK(const std::string& name, const CreateInfos& create_infos) : color_attachment_count(static_cast<uint32_t>(create_infos.render_pass_config.color_attachments.size()))
{
    std::vector<VkAttachmentDescription> attachment_descriptions;
    std::vector<VkAttachmentReference>   color_attachment_references;
    std::optional<VkAttachmentReference> depth_attachment_reference;
    std::optional<VkAttachmentReference> color_attachment_resolve_reference;

    // add color color_attachments
    for (const auto& col_attachment : create_infos.render_pass_config.color_attachments)
    {
        if (col_attachment.image_format == ETypeFormat::UNDEFINED)
            LOG_FATAL("images buffer format is undefined");

        const uint32_t attachment_index = static_cast<uint32_t>(attachment_descriptions.size());

        attachment_descriptions.emplace_back(VkAttachmentDescription{
            .format         = vk_texture_format_to_engine(col_attachment.image_format),
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = col_attachment.clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = create_infos.in_present_pass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });

        color_attachment_references.emplace_back(VkAttachmentReference{
            .attachment = attachment_index,
            .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        });
    }

    // add depth attachment
    if (create_infos.render_pass_config.depth_attachment)
    {
        if (create_infos.render_pass_config.depth_attachment->image_format == ETypeFormat::UNDEFINED)
            LOG_FATAL("images buffer format is undefined");

        const uint32_t attachment_index = static_cast<uint32_t>(attachment_descriptions.size());

        attachment_descriptions.emplace_back(VkAttachmentDescription{
            .format         = vk_texture_format_to_engine(create_infos.render_pass_config.depth_attachment->image_format),
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = create_infos.render_pass_config.depth_attachment->clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
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
    debug_set_object_name(name, render_pass);
}

RenderPassResource_VK::~RenderPassResource_VK()
{
    vkDestroyRenderPass(get_device(), render_pass, get_allocator());
}

FramebufferResource_VK::FramebufferResource_VK(const std::string& name, const CreateInfos& create_infos) : framebuffer_views(create_infos.framebuffer_views), render_pass(create_infos.render_pass)
{
    std::vector<VkImageView> attachments(0);
    for (const auto& view : create_infos.framebuffer_views)
        attachments.emplace_back(view->view);

    const VkFramebufferCreateInfo framebuffer_infos{
        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass      = create_infos.render_pass->render_pass,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments    = attachments.data(),
        .width           = create_infos.width,
        .height          = create_infos.height,
        .layers          = 1,
    };

    VK_CHECK(vkCreateFramebuffer(get_device(), &framebuffer_infos, get_allocator(), &framebuffer), "Failed to create framebuffers");
    debug_set_object_name(name, framebuffer);
}

FramebufferResource_VK::~FramebufferResource_VK()
{
    vkDestroyFramebuffer(get_device(), framebuffer, get_allocator());
}
} // namespace gfx::vulkan