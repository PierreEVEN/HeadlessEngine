
#include "vk_render_pass_instance.h"

#include "gfx/physical_device.h"
#include "vk_device.h"
#include "vk_physical_device.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_command_buffer.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_render_pass.h"
#include "vulkan/vk_texture.h"

namespace gfx::vulkan
{

VkClearValue to_vk_clear_color(const ClearValue& in_clear)
{
    VkClearValue clear_value;
    clear_value.color.float32[0] = in_clear.color[0];
    clear_value.color.float32[1] = in_clear.color[1];
    clear_value.color.float32[2] = in_clear.color[2];
    clear_value.color.float32[3] = in_clear.color[3];
    return clear_value;
}
VkClearValue to_vk_clear_depth_stencil(const ClearValue& in_clear)
{
    VkClearValue clear_value;
    clear_value.depthStencil.depth   = in_clear.depth;
    clear_value.depthStencil.stencil = in_clear.stencil;
    return clear_value;
}

FramebufferResource_VK::FramebufferResource_VK(const std::string& name, const CI_Framebuffer& create_infos) : parameters(create_infos)
{
    std::vector<VkImageView> attachments(0);
    for (const auto& image : create_infos.images)
        attachments.emplace_back(image->image);

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

RenderPassInstance_VK::RenderPassInstance_VK(uint32_t width, uint32_t height, const RenderPassID& base, const std::vector<std::shared_ptr<Texture>>& images) : RenderPassInstance(width, height, base, images)
{
    resize(width, height, images);

    const VkSemaphoreCreateInfo semaphore_infos{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    for (auto& data : frame_data)
    {
        data.render_finished_semaphore = TGpuHandle<SemaphoreResource_VK>("sem test", SemaphoreResource_VK::CI_Semaphore{});
    }
}

void RenderPassInstance_VK::begin_pass()
{
    // Begin get record
    get_pass_command_buffer()->start();

    const auto*            base = dynamic_cast<RenderPass_VK*>(get_base());
    const VkCommandBuffer& cmd  = **dynamic_cast<CommandBuffer_VK*>(get_pass_command_buffer());

    debug_add_marker("draw render pass [" + base->get_config().pass_name + "]", cmd, {0.5f, 1.0f, 0.5f, 1.0f});

    // Begin render pass
    std::vector<VkClearValue> clear_values;
    for (auto& attachment : get_base()->get_config().color_attachments)
        clear_values.emplace_back(attachment.clear_value ? to_vk_clear_color(attachment.clear_value.value()) : VkClearValue{});

    if (get_base()->get_config().depth_attachment)
        clear_values.emplace_back(get_base()->get_config().depth_attachment->clear_value ? to_vk_clear_depth_stencil(get_base()->get_config().depth_attachment->clear_value.value()) : VkClearValue{});

    const VkRenderPassBeginInfo begin_infos = {
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass  = base->get()->render_pass,
        .framebuffer = frame_data->framebuffer->framebuffer,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = {get_width(), get_height()},
            },
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues    = clear_values.data(),
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

    // End command get
    vkCmdEndRenderPass(cmd);
    debug_end_marker(cmd);

    // End get record
    get_pass_command_buffer()->end();

    // Submit get (wait children completion using children_semaphores)
    std::vector<VkSemaphore> children_semaphores;
    for (const auto& child : children)
        children_semaphores.emplace_back(*dynamic_cast<RenderPassInstance_VK*>(child.get())->frame_data->render_finished_semaphore->semaphore);
    if (get_base()->is_present_pass())
        children_semaphores.emplace_back(swapchain_image_acquire_semaphore);
    std::vector<VkPipelineStageFlags> wait_stage(children_semaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    const VkSubmitInfo                submit_infos{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = static_cast<uint32_t>(children_semaphores.size()),
        .pWaitSemaphores      = children_semaphores.data(),
        .pWaitDstStageMask    = wait_stage.data(),
        .commandBufferCount   = 1,
        .pCommandBuffers      = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &frame_data->render_finished_semaphore->semaphore,
    };
    get_physical_device<PhysicalDevice_VK>()->submit_queue(EQueueFamilyType::GRAPHIC_QUEUE, submit_infos);
}

void RenderPassInstance_VK::resize(uint32_t width, uint32_t height, const std::vector<std::shared_ptr<Texture>>& surface_texture)
{
    std::vector<TGpuHandle<ImageResource_VK>> images;


    for (const auto& image : framebuffers_images)
        images.emplace_back(static_cast<Texture_VK*>(image.get())->)

    for (auto& frame : frame_data)
        frame.framebuffer = TGpuHandle<FramebufferResource_VK>("framebuffer", FramebufferResource_VK::CI_Framebuffer{
                                                                                  .width       = width,
                                                                                  .height      = height,
                                                                                  .render_pass = render_pass,
                                                                                  .images      = {},
                                                                              });
}
} // namespace gfx::vulkan