
#include "vk_render_pass_instance.h"

#include "gfx/physical_device.h"
#include "vk_physical_device.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_command_buffer.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_render_pass.h"
#include "vulkan/vk_texture.h"

namespace gfx::vulkan
{
RenderPassInstance_VK::RenderPassInstance_VK(uint32_t width, uint32_t height, const RenderPassID& base, const std::optional<std::vector<std::shared_ptr<Texture>>>& images)
    : RenderPassInstance(width, height, base, images)
{
    resize(width, height);

    const VkSemaphoreCreateInfo semaphore_infos{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    for (auto& semaphore : render_finished_semaphore)
        vkCreateSemaphore(get_device(), &semaphore_infos, get_allocator(), &semaphore);
}

RenderPassInstance_VK::~RenderPassInstance_VK()
{
    for (const auto& framebuffer : framebuffers)
        vkDestroyFramebuffer(get_device(), framebuffer, get_allocator());

    for (const auto& semaphore : render_finished_semaphore)
        vkDestroySemaphore(get_device(), semaphore, get_allocator());
}

void RenderPassInstance_VK::begin_pass()
{
    // Begin buffer record
    get_pass_command_buffer()->start();

    const auto*            base = dynamic_cast<RenderPass_VK*>(get_base());
    const VkCommandBuffer& cmd  = **dynamic_cast<CommandBuffer_VK*>(get_pass_command_buffer());

    debug_add_marker("draw render pass [" + base->get_config().pass_name + "]", cmd, {0.5f, 1.0f, 0.5f, 1.0f});

    // Begin render pass @TODO : add clear values
    const VkRenderPassBeginInfo begin_infos = {
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass  = base->get(),
        .framebuffer = *framebuffers,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = {get_width(), get_height()},
            },
    };
    vkCmdBeginRenderPass(cmd, &begin_infos, VK_SUBPASS_CONTENTS_INLINE);

    // Set viewport and scissor
    const VkViewport viewport{
        .x        = 0,
        .y        = static_cast<float>(get_height()), // Flip viewport vertically to avoid textures to being displayed upside down
        .width    = static_cast<float>(get_width()),
        .height   = -static_cast<float>(get_height()),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    const VkRect2D scissor{
        .offset = VkOffset2D{0, 0},
        .extent = VkExtent2D{get_width(), get_height()},
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void RenderPassInstance_VK::submit()
{
    const VkCommandBuffer& cmd = **dynamic_cast<CommandBuffer_VK*>(get_pass_command_buffer());

    // End command buffer
    vkCmdEndRenderPass(cmd);
    debug_end_marker(cmd);

    // End buffer record
    get_pass_command_buffer()->end();

    // Submit buffer (wait children completion using children_semaphores)
    std::vector<VkSemaphore> children_semaphores;
    for (const auto& child : children)
        children_semaphores.emplace_back(*dynamic_cast<RenderPassInstance_VK*>(child.get())->render_finished_semaphore);
    if (get_base()->is_present_pass())
        children_semaphores.emplace_back(swapchain_image_acquire_semaphore);
    std::vector<VkPipelineStageFlags> wait_stage(children_semaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    const VkSubmitInfo   submit_infos{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = static_cast<uint32_t>(children_semaphores.size()),
        .pWaitSemaphores      = children_semaphores.data(),
        .pWaitDstStageMask    = wait_stage.data(),
        .commandBufferCount   = 1,
        .pCommandBuffers      = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &*render_finished_semaphore,
    };
    get_physical_device<PhysicalDevice_VK>()->submit_queue(EQueueFamilyType::GRAPHIC_QUEUE, submit_infos);
}

void RenderPassInstance_VK::resize(uint32_t width, uint32_t height)
{
    RenderPass_VK* base = static_cast<RenderPass_VK*>(get_base());

    framebuffer_width  = width;
    framebuffer_height = height;
    for (uint8_t i = 0; i < framebuffers.get_max_instance_count(); ++i)
    {
        std::vector<VkImageView> attachments(0);
        for (const auto& image : get_framebuffer_images())
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