#include "rendering/renderer/render_pass.h"

#include "rendering/graphics.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/utils.h"
#include <cpputils/logger.hpp>

RenderPass::RenderPass(const RenderPassSettings& in_pass_description)
{
    pass_description = in_pass_description;
    create_render_pass();
}

RenderPass::~RenderPass()
{
    destroy_render_pass();
}

void RenderPass::create_render_pass()
{
    std::vector<VkAttachmentDescription> attachment_descriptions;
    std::vector<VkAttachmentReference>   color_attachment_references;
    std::optional<VkAttachmentReference> depth_attachment_reference;
    std::optional<VkAttachmentReference> color_attachment_resolve_reference;

    // add color attachments
    for (const auto& col_attachment : pass_description.color_attachments)
    {
        if (col_attachment.image_format == VK_FORMAT_UNDEFINED)
            LOG_FATAL("image buffer format is undefined");

        const uint32_t attachment_index = static_cast<uint32_t>(attachment_descriptions.size());

        attachment_descriptions.emplace_back(VkAttachmentDescription{
            .format         = col_attachment.image_format,
            .samples        = pass_description.sample_count,
            .loadOp         = col_attachment.clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = pass_description.b_use_swapchain_image && !pass_description.has_resolve_attachment() ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });

        color_attachment_references.emplace_back(VkAttachmentReference{
            .attachment = attachment_index,
            .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        });
    }

    // add depth attachment
    if (pass_description.depth_attachment)
    {
        const uint32_t attachment_index = static_cast<uint32_t>(attachment_descriptions.size());

        attachment_descriptions.emplace_back(VkAttachmentDescription{
            .format         = pass_description.depth_attachment->image_format,
            .samples        = pass_description.sample_count,
            .loadOp         = pass_description.depth_attachment->clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
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

    // add resolve attachment
    if (pass_description.has_resolve_attachment())
    {
        const uint32_t attachment_index = static_cast<uint32_t>(attachment_descriptions.size());

        if (pass_description.get_resolve_format() == VK_FORMAT_UNDEFINED)
            LOG_FATAL("resolve format is undefined");

        attachment_descriptions[attachment_index] = VkAttachmentDescription{
            .format         = pass_description.get_resolve_format(),
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // don't need to clear resolve attachment
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };
        color_attachment_resolve_reference = VkAttachmentReference{
            .attachment = attachment_index,
            .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }

    const VkSubpassDescription subpass{
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount    = 0,       // Input attachments can be used to sample from contents of a previous subpass
        .pInputAttachments       = nullptr, // (Input attachments not used here)
        .colorAttachmentCount    = static_cast<uint32_t>(color_attachment_references.size()),
        .pColorAttachments       = color_attachment_references.data(),
        .pResolveAttachments     = color_attachment_resolve_reference ? &color_attachment_resolve_reference.value() : nullptr, // resolve mean the target attachment for msaa
        .pDepthStencilAttachment = depth_attachment_reference ? &depth_attachment_reference.value() : nullptr,
        .preserveAttachmentCount = 0,       // Preserved attachments can be used to loop (and preserve) attachments through subpasses
        .pPreserveAttachments    = nullptr, // (Preserve attachments not used by this example)
    };

    const std::array dependencies{VkSubpassDependency{
                                      .srcSubpass      = VK_SUBPASS_EXTERNAL,                                                        // Producer of the dependency
                                      .dstSubpass      = 0,                                                                          // Consumer is our single subpass that will wait for the execution depdendency
                                      .srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                       // Match our pWaitDstStageMask when we vkQueueSubmit
                                      .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                              // is a loadOp stage for color attachments
                                      .srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT,                                                  // semaphore wait already does memory dependency for us
                                      .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // is a loadOp CLEAR access mask for color attachments
                                      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                                  },
                                  VkSubpassDependency{
                                      .srcSubpass      = 0,                                                                          // Producer of the dependency is our single subpass
                                      .dstSubpass      = VK_SUBPASS_EXTERNAL,                                                        // Consumer are all commands outside of the renderpass
                                      .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                              // is a storeOp stage for color attachments
                                      .dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                       // Do not block any subsequent work
                                      .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // is a storeOp `STORE` access mask for color attachments
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

    VK_ENSURE(vkCreateRenderPass(Graphics::get()->get_logical_device(), &render_pass_infos, vulkan_common::allocation_callback, &render_pass), "Failed to create render pass");
}

void RenderPass::destroy_render_pass()
{
    if (render_pass != VK_NULL_HANDLE)
        vkDestroyRenderPass(Graphics::get()->get_logical_device(), render_pass, vulkan_common::allocation_callback);
    render_pass = VK_NULL_HANDLE;
}