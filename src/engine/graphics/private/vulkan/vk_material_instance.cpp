#include "vk_material_instance.h"

#include "gfx/buffer.h"
#include "gfx/command_buffer.h"
#include "gfx/render_pass.h"
#include "vk_buffer.h"
#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_material.h"
#include "vk_render_pass.h"

namespace gfx::vulkan
{
MaterialInstance_VK::MaterialInstance_VK(const std::shared_ptr<MasterMaterial>& base) : MaterialInstance(base)
{
    const auto* vk_base = dynamic_cast<MasterMaterial_VK*>(get_base().get());

    for (auto it = get_base()->get_vertex_reflections().begin(); it != get_base()->get_vertex_reflections().end(); ++it)
    {
        const auto&                 desc_set_layouts = vk_base->get_descriptor_set_layout(it.id());
        VkDescriptorSetAllocateInfo descriptor_info{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext              = nullptr,
            .descriptorPool     = VK_NULL_HANDLE,
            .descriptorSetCount = 1,
            .pSetLayouts        = &desc_set_layouts,
        };
        dynamic_cast<RenderPass_VK*>(RenderPass::find(it.id()))->get_descriptor_pool().alloc_memory(descriptor_info);
        auto& desc_sets = descriptor_sets.init(it.id());
        vkAllocateDescriptorSets(get_device(), &descriptor_info, &desc_sets.descriptor_set);
        for (auto& write_desc_set : desc_sets.write_descriptor_sets)
            write_desc_set.is_dirty = true;
    }
}

void MaterialInstance_VK::bind_buffer(const std::string& binding_name, const std::shared_ptr<Buffer>& in_buffer)
{
    if (!in_buffer)
        return;
    write_buffers[binding_name] = in_buffer;
    for (auto& pass : descriptor_sets)
        for (auto& image : pass.write_descriptor_sets)
            image.is_dirty = true;
}

void MaterialInstance_VK::bind_material(CommandBuffer* command_buffer)
{
    if (!descriptor_sets.contains(command_buffer->get_render_pass()))
        return;

    const auto            base            = dynamic_cast<MasterMaterial_VK*>(get_base().get());
    const auto*           pipeline_layout = base->get_pipeline_layout(command_buffer->get_render_pass());
    const auto*           pipeline        = base->get_pipeline(command_buffer->get_render_pass());
    const VkCommandBuffer cmd             = **dynamic_cast<CommandBuffer_VK*>(command_buffer);

    if (!pipeline_layout)
        return;

    if (!pipeline)
        return;

    if (base->get_properties().line_width != 1.0f)
        vkCmdSetLineWidth(cmd, base->get_properties().line_width);

    auto& pass_data = descriptor_sets[command_buffer->get_render_pass()];

    if (pass_data.write_descriptor_sets->is_dirty)
    {
        auto& desc_set = pass_data.write_descriptor_sets->write_descriptor_sets;
        desc_set       = std::vector<VkWriteDescriptorSet>();

        for (const auto& buffer : write_buffers)
        {
            const auto* binding = find_binding(buffer.first, command_buffer->get_render_pass());
            if (!binding)
                continue;
            desc_set.emplace_back(VkWriteDescriptorSet{
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = pass_data.descriptor_set,
                .dstBinding       = binding->binding,
                .dstArrayElement  = 0,
                .descriptorCount  = 1,
                .descriptorType   = MasterMaterial_VK::vk_descriptor_type(binding->descriptor_type),
                .pImageInfo       = nullptr,
                .pBufferInfo      = &dynamic_cast<Buffer_VK*>(buffer.second.get())->get_buffer_infos(),
                .pTexelBufferView = nullptr,
            });
        }

        vkUpdateDescriptorSets(get_device(), static_cast<uint32_t>(pass_data.write_descriptor_sets->write_descriptor_sets.size()), pass_data.write_descriptor_sets->write_descriptor_sets.data(), 0, nullptr);

        pass_data.write_descriptor_sets->is_dirty = false;
    }
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline_layout, 0, 1, &pass_data.descriptor_set, 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
}

const shader_builder::BindingDescriptor* MaterialInstance_VK::find_binding(const std::string& binding_name, const RenderPassID& render_pass) const
{
    for (auto& binding : get_base()->get_vertex_reflection(render_pass).bindings)
        if (binding.name == binding_name)
            return &binding;

    for (auto& binding : get_base()->get_fragment_reflection(render_pass).bindings)
        if (binding.name == binding_name)
            return &binding;

    return nullptr;
}

void MaterialInstance_VK::push_constants_internal(CommandBuffer* command_buffer, bool is_vertex_stage, const void* data, size_t data_size)
{
}
} // namespace gfx::vulkan