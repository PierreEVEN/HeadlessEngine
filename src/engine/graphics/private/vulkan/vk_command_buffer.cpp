
#include "vk_command_buffer.h"

#include "gfx/buffer.h"
#include "vk_buffer.h"
#include "vk_helper.h"
#include "vk_material.h"
#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_material_instance.h"

#include "gfx/StaticMesh.h"

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
        VK_CHECK(vkAllocateCommandBuffers(get_device(), &command_buffer_infos, &buffer), "Failed to allocate command buffer");
        debug_set_object_name(stringutils::format("command buffer %s #%d", name.c_str(), cmd++), buffer);
    }
}

CommandBuffer_VK::~CommandBuffer_VK()
{
    vkDeviceWaitIdle(get_device());
    for (auto& buffer : command_buffer)
        vkFreeCommandBuffers(get_device(), command_pool::get(), 1, &buffer);
}

void CommandBuffer_VK::bind_material(MaterialInstance* in_material)
{
    in_material->bind_material(this);
}

void CommandBuffer_VK::draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    const auto& cmd = *command_buffer;
    bind_material(in_material);
    vkCmdDraw(cmd, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer_VK::draw_mesh(StaticMesh* in_buffer, MaterialInstance* in_material)
{
    const auto& cmd = *command_buffer;
    bind_material(in_material);
    dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
    dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
    vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
}

void CommandBuffer_VK::draw_mesh_indirect(StaticMesh* in_buffer, MaterialInstance* in_material)
{
    const auto& cmd = *command_buffer;
    bind_material(in_material);
    dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
    dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
    vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
}

void CommandBuffer_VK::draw_mesh_instanced(StaticMesh* in_buffer, MaterialInstance* in_material)
{
    const auto& cmd = *command_buffer;
    bind_material(in_material);
    dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
    dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
    vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
}

void CommandBuffer_VK::draw_mesh_instanced_indirect(StaticMesh* in_buffer, MaterialInstance* in_material)
{
    const auto& cmd = *command_buffer;
    bind_material(in_material);
    dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
    dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
    vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
}

void CommandBuffer_VK::set_scissor(const Scissor& scissors)
{
    const VkRect2D vk_scissor{
        .extent =
            VkExtent2D{
                .width  = scissors.width,
                .height = scissors.height,
            },
        .offset =
            VkOffset2D{
                .x = scissors.offset_x,
                .y = scissors.offset_y,
            },
    };
    vkCmdSetScissor(*command_buffer, 0, 1, &vk_scissor);
}

void CommandBuffer_VK::start()
{
    const VkCommandBufferBeginInfo begin_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags            = 0,
        .pInheritanceInfo = nullptr,
    };
    VK_CHECK(vkBeginCommandBuffer(*command_buffer, &begin_info), "Failed to start command buffer");
}

void CommandBuffer_VK::end()
{
    VK_CHECK(vkEndCommandBuffer(*command_buffer), "Failed to end command buffer");
}
} // namespace gfx::vulkan