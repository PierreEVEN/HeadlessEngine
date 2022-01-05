
#include "vk_command_buffer.h"

#include "vk_helper.h"
#include "vk_material.h"
#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
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

void CommandBuffer_VK::draw_procedural(MaterialInstance* in_material, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    MasterMaterial_VK* base = dynamic_cast<MasterMaterial_VK*>(in_material->get_base().get());

    const auto* pipeline_layout = base->get_pipeline_layout(render_pass);
    const auto* pipeline = base->get_pipeline(render_pass);

    if (!pipeline_layout)
        return;

    if (!pipeline)
        return;
    //update_descriptor_sets(render_context.render_pass, render_context.view, render_context.image_index);

    //vkCmdBindDescriptorSets(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline_layout, 0, 1,&, 0, nullptr);

    if (base->get_properties().line_width != 1.0f)
        vkCmdSetLineWidth(*command_buffer, base->get_properties().line_width);

    vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
    vkCmdDraw(*command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer_VK::draw_mesh(Mesh* in_buffer, MaterialInstance* in_material, RenderLayer render_layer)
{
    (void)in_buffer;
    (void)in_material;
    (void)render_layer;
}

void CommandBuffer_VK::draw_mesh_indirect(Mesh* in_buffer, MaterialInstance* in_material, RenderLayer render_layer)
{
    (void)in_buffer;
    (void)in_material;
    (void)render_layer;
}

void CommandBuffer_VK::draw_mesh_instanced(Mesh* in_buffer, MaterialInstance* in_material, RenderLayer render_layer)
{
    (void)in_buffer;
    (void)in_material;
    (void)render_layer;
}

void CommandBuffer_VK::draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInstance* in_material, RenderLayer render_layer)
{
    (void)in_buffer;
    (void)in_material;
    (void)render_layer;
}
} // namespace gfx::vulkan