
#include "vk_command_buffer.h"

#include "gfx/Mesh.h"
#include "gfx/buffer.h"
#include "vulkan/vk_buffer.h"
#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_master_material.h"
#include "vulkan/vk_material_instance.h"

namespace gfx::vulkan
{
CommandBuffer_VK::CommandBuffer_VK(const std::string& name)
{
    const VkCommandBufferAllocateInfo command_buffer_infos{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = command_pool::get(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    uint8_t cmd = 0;
    for (auto& buffer : command_buffer)
    {
        VK_CHECK(vkAllocateCommandBuffers(get_device(), &command_buffer_infos, &buffer), "Failed to allocate command get");
        debug_set_object_name(stringutils::format("command_buffer:%s:frame=%d", name.c_str(), cmd++), buffer);
    }
}

CommandBuffer_VK::~CommandBuffer_VK()
{
    vkDeviceWaitIdle(get_device());
    for (auto& buffer : command_buffer)
        vkFreeCommandBuffers(get_device(), command_pool::get(), 1, &buffer);
}

void CommandBuffer_VK::draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    const auto& cmd = *command_buffer;
    if (in_material->bind_material(this))
        vkCmdDraw(cmd, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer_VK::draw_mesh(Mesh* in_buffer, MaterialInstance* in_material, uint32_t instance_count, uint32_t first_instance)
{
    const auto& cmd = *command_buffer;
    if (in_material->bind_material(this))
    {
        dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
        dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
        vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), instance_count, 0, 0, first_instance);
    }
}

void CommandBuffer_VK::draw_mesh(Mesh* in_buffer, MaterialInstance* in_material, uint32_t first_index, uint32_t vertex_offset, uint32_t index_count, uint32_t instance_count, uint32_t first_instance)
{
    const auto& cmd = *command_buffer;
    if (in_material->bind_material(this))
    {
        dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
        dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
        vkCmdDrawIndexed(cmd, std::min(index_count, in_buffer->get_index_buffer()->count() - first_index), instance_count, first_index, vertex_offset, first_instance);
    }
}

void CommandBuffer_VK::draw_mesh_indirect(Mesh* in_buffer, MaterialInstance* in_material)
{
    const auto& cmd = *command_buffer;
    if (in_material->bind_material(this))
    {
        dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
        dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
        vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
    }
}

void CommandBuffer_VK::set_scissor(const Scissor& scissors)
{
    const VkRect2D vk_scissor{
        .offset =
            VkOffset2D{
                .x = scissors.offset_x,
                .y = scissors.offset_y,
            },
        .extent =
            VkExtent2D{
                .width  = scissors.width,
                .height = scissors.height,
            },
    };
    vkCmdSetScissor(*command_buffer, 0, 1, &vk_scissor);
}

void CommandBuffer_VK::push_constant(bool is_vertex_buffer, const MaterialInstance* material, const void* data, uint32_t data_size)
{
    const auto* material_base = static_cast<MasterMaterial_VK*>(material->get_base().get());

    if (is_vertex_buffer)
    {
        const auto& push_constant = material_base->get_vertex_reflection(*render_pass).push_constant;
        if (!push_constant)
        {
            LOG_ERROR("vertex stage does not have push constants");
            return;
        }
        if (push_constant->structure_size != data_size)
        {
            LOG_ERROR("wrong push constant data size : %d Expected %d", data_size, push_constant->structure_size);
            return;
        }
    }
    else
    {
        const auto& push_constant = material_base->get_fragment_reflection(*render_pass).push_constant;
        if (!push_constant)
        {
            LOG_ERROR("framgent stage does not have push constants");
            return;
        }
        if (push_constant->structure_size != data_size)
        {
            LOG_ERROR("wrong push constant data size : %d Expected %d", data_size, push_constant->structure_size);
            return;
        }
    }

    vkCmdPushConstants(*command_buffer, material_base->get_pipeline_layout(*render_pass)->pipeline_layout, is_vertex_buffer ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT, 0, data_size, data);
}

void CommandBuffer_VK::start()
{
    const VkCommandBufferBeginInfo begin_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags            = 0,
        .pInheritanceInfo = nullptr,
    };
    VK_CHECK(vkBeginCommandBuffer(*command_buffer, &begin_info), "Failed to start command get");
}

void CommandBuffer_VK::end()
{
    VK_CHECK(vkEndCommandBuffer(*command_buffer), "Failed to end command get");
}

CommandBufferResource_VK::CommandBufferResource_VK(const std::string& name, const CI_CommandBuffer&) : command_buffer(VK_NULL_HANDLE)
{
    const VkCommandBufferAllocateInfo command_buffer_infos{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = command_pool::get(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VK_CHECK(vkAllocateCommandBuffers(get_device(), &command_buffer_infos, &command_buffer), "Failed to allocate command get");
    debug_set_object_name(stringutils::format("%s", name.c_str()), command_buffer);
}

CommandBufferResource_VK::~CommandBufferResource_VK()
{
    vkFreeCommandBuffers(get_device(), command_pool::get(), 1, &command_buffer);
}
} // namespace gfx::vulkan