
#include "vk_command_buffer.h"

#include "gfx/buffer.h"
#include "vk_buffer.h"
#include "vk_helper.h"
#include "vk_material.h"
#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_material_instance.h"

#include "gfx/mesh.h"

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
        for (auto& pass_id : RenderPassID::get_all())
        {
            auto& pass_buffer = buffer.init(pass_id);
            VK_CHECK(vkAllocateCommandBuffers(get_device(), &command_buffer_infos, &pass_buffer), "Failed to allocate command buffer");
            debug_set_object_name(stringutils::format("command buffer %s #%d %s", name.c_str(), cmd++, pass_id.name().c_str()), pass_buffer);
        }
    }
}

CommandBuffer_VK::~CommandBuffer_VK()
{
    vkDeviceWaitIdle(get_device());
    for (auto& buffer : command_buffer)
    {
        for (const auto& pass_buffer : buffer)
            vkFreeCommandBuffers(get_device(), command_pool::get(), 1, &pass_buffer);
        buffer.clear();
    }
}

void CommandBuffer_VK::bind_material(VkCommandBuffer cmd, MaterialInstance* in_material)
{
    const auto base = dynamic_cast<MasterMaterial_VK*>(in_material->get_base().get());

    const auto* pipeline_layout = base->get_pipeline_layout(*render_pass);
    const auto* pipeline        = base->get_pipeline(*render_pass);

    if (!pipeline_layout)
        return;

    if (!pipeline)
        return;

    if (base->get_properties().line_width != 1.0f)
        vkCmdSetLineWidth(cmd, base->get_properties().line_width);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
}

void CommandBuffer_VK::draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    for (const auto& pass : in_material->get_compatible_render_passes())
    {
        const auto& cmd = (*command_buffer)[pass];
        bind_material(cmd, in_material);
        vkCmdDraw(cmd, vertex_count, instance_count, first_vertex, first_instance);
    }
}

void CommandBuffer_VK::draw_mesh(Mesh* in_buffer, MaterialInstance* in_material)
{
    for (const auto& pass : in_material->get_compatible_render_passes())
    {
        const auto& cmd = (*command_buffer)[pass];
        bind_material(cmd, in_material);
        dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
        dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
        vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
    }
}

void CommandBuffer_VK::draw_mesh_indirect(Mesh* in_buffer, MaterialInstance* in_material)
{
    for (const auto& pass : in_material->get_compatible_render_passes())
    {
        const auto& cmd = (*command_buffer)[pass];
        bind_material(cmd, in_material);
        dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
        dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
        vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
    }
}

void CommandBuffer_VK::draw_mesh_instanced(Mesh* in_buffer, MaterialInstance* in_material)
{
    for (const auto& pass : in_material->get_compatible_render_passes())
    {
        const auto& cmd = (*command_buffer)[pass];
        bind_material(cmd, in_material);
        dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
        dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
        vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
    }
}

void CommandBuffer_VK::draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInstance* in_material)
{
    for (const auto& pass : in_material->get_compatible_render_passes())
    {
        const auto& cmd = (*command_buffer)[pass];
        bind_material(cmd, in_material);
        dynamic_cast<Buffer_VK*>(in_buffer->get_vertex_buffer())->bind_buffer(cmd);
        dynamic_cast<Buffer_VK*>(in_buffer->get_index_buffer())->bind_buffer(cmd);
        vkCmdDrawIndexed(cmd, in_buffer->get_index_buffer()->count(), 1, 0, 0, 0);
    }
}
} // namespace gfx::vulkan