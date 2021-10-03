#include "rendering/renderer/render_pass.h"

#include "rendering/gfx_context.h"
#include "rendering/renderer/swapchain.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/utils.h"
#include <cpputils/logger.hpp>

RenderPass::RenderPass(Swapchain* target_swapchain) : swapchain(target_swapchain)
{
    create_or_recreate_render_pass();
}

RenderPass::~RenderPass()
{
    if (render_pass != VK_NULL_HANDLE)
    {
        destroy_render_pass();
    }
}

void RenderPass::create_or_recreate_render_pass()
{
    if (render_pass != VK_NULL_HANDLE)
    {
        destroy_render_pass();
    }

    LOG_INFO("Create render pass");
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format         = swapchain->get_surface_format().format;
    colorAttachment.samples        = static_cast<VkSampleCountFlagBits>(vulkan_common::get_msaa_sample_count());
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = vulkan_common::get_msaa_sample_count() > 1 ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format         = vulkan_utils::get_depth_format(GfxContext::get()->physical_device);
    depthAttachment.samples        = static_cast<VkSampleCountFlagBits>(vulkan_common::get_msaa_sample_count());
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format         = swapchain->get_surface_format().format;
    colorAttachmentResolve.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments     = vulkan_common::get_msaa_sample_count() > 1 ? &colorAttachmentResolveRef : nullptr;
    subpass.inputAttachmentCount    = 0;       // Input attachments can be used to sample from contents of a previous subpass
    subpass.pInputAttachments       = nullptr; // (Input attachments not used by this example)
    subpass.preserveAttachmentCount = 0;       // Preserved attachments can be used to loop (and preserve) attachments through subpasses
    subpass.pPreserveAttachments    = nullptr; // (Preserve attachments not used by this example)

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;                           // Producer of the dependency
    dependencies[0].dstSubpass      = 0;                                             // Consumer is our single subpass that will wait for the execution depdendency
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Match our pWaitDstStageMask when we vkQueueSubmit
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a loadOp stage for color attachments
    dependencies[0].srcAccessMask   = 0;                                             // semaphore wait already does memory dependency for us
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          // is a loadOp CLEAR access mask for color attachments
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass      = 0;                                             // Producer of the dependency is our single subpass
    dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;                           // Consumer are all commands outside of the renderpass
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a storeOp stage for color attachments
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;          // Do not block any subsequent work
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          // is a storeOp `STORE` access mask for color attachments
    dependencies[1].dstAccessMask   = 0;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::vector<VkAttachmentDescription> attachments;

    if (vulkan_common::get_msaa_sample_count() > 1)
    {
        attachments.push_back(colorAttachment);
        attachments.push_back(depthAttachment);
        attachments.push_back(colorAttachmentResolve);
    }
    else
    {
        attachments.push_back(colorAttachment);
        attachments.push_back(depthAttachment);
    }
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies   = dependencies.data();
    VK_ENSURE(vkCreateRenderPass(GfxContext::get()->logical_device, &renderPassInfo, vulkan_common::allocation_callback, &render_pass), "Failed to create render pass");
}

void RenderPass::destroy_render_pass()
{
    LOG_INFO("Destroy Render pass");
    vkDestroyRenderPass(GfxContext::get()->logical_device, render_pass, vulkan_common::allocation_callback);
    render_pass = VK_NULL_HANDLE;
}