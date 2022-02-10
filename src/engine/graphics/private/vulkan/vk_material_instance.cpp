#include "vk_material_instance.h"

#include "gfx/buffer.h"
#include "gfx/command_buffer.h"
#include "gfx/render_pass.h"
#include "vk_buffer.h"
#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_errors.h"
#include "vk_master_material.h"
#include "vk_render_pass.h"
#include "vk_sampler.h"
#include "vk_texture.h"
#include <vulkan/vk_helper.h>

namespace gfx::vulkan
{
DescriptorSetResource_VK::DescriptorSetResource_VK(const std::string& name, const CI_DescriptorSet& create_infos) : parameters(create_infos)
{
    VkDescriptorSetAllocateInfo descriptor_info{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = VK_NULL_HANDLE,
        .descriptorSetCount = 1,
        .pSetLayouts        = &create_infos.desc_set_layout->descriptor_set_layout,
    };
    create_infos.descriptor_pool.alloc_memory(descriptor_info);
    VK_CHECK(vkAllocateDescriptorSets(get_device(), &descriptor_info, &descriptor_set), "failed to allocate descriptor sets");
    debug_set_object_name(name, descriptor_set);
    is_dirty = true;
}

DescriptorSetResource_VK::~DescriptorSetResource_VK()
{
    //@TODO : free descriptor sets
}

MaterialInstance_VK::MaterialInstance_VK(const std::shared_ptr<MasterMaterial>& base) : MaterialInstance(base)
{

    const auto* vk_base = dynamic_cast<MasterMaterial_VK*>(get_base().get());

    for (auto it = get_base()->get_vertex_reflections().begin(); it != get_base()->get_vertex_reflections().end(); ++it)
    {
        const auto& desc_set_layouts = vk_base->get_descriptor_set_layouts(it.id());
        auto&       desc_sets        = descriptor_sets.init(it.id());
        for (auto& desc_set : desc_sets)
        {
            desc_set = TGpuHandle<DescriptorSetResource_VK>("desc set test", DescriptorSetResource_VK::CI_DescriptorSet{
                                                                                 .desc_set_layout = desc_set_layouts,
                                                                                 .descriptor_pool = dynamic_cast<RenderPass_VK*>(RenderPass::find(it.id()))->get_descriptor_pool(),
                                                                             });
        }
    }
}

void MaterialInstance_VK::bind_buffer(const std::string& binding_name, const std::shared_ptr<Buffer>& in_buffer)
{
    if (!in_buffer)
    {
        LOG_WARNING("trying to bind a null get to %s", binding_name.c_str());
        return;
    }
    write_buffers[binding_name] = in_buffer;
    for (auto& pass : descriptor_sets)
        for (auto& image : pass)
            image->is_dirty = true;
}

void MaterialInstance_VK::bind_texture(const std::string& binding_name, const std::shared_ptr<Texture>& in_texture)
{
    if (!in_texture)
        return;
    write_textures[binding_name] = in_texture;
    for (auto& pass : descriptor_sets)
        for (auto& image : pass)
            image->is_dirty = true;
}

void MaterialInstance_VK::bind_sampler(const std::string& binding_name, const std::shared_ptr<Sampler>& in_sampler)
{
    if (!in_sampler)
        return;
    write_samplers[binding_name] = in_sampler;
    for (auto& pass : descriptor_sets)
        for (auto& image : pass)
            image->is_dirty = true;
}

bool MaterialInstance_VK::bind_material(CommandBuffer* command_buffer)
{
    if (!descriptor_sets.contains(command_buffer->get_render_pass()))
        return false;

    const auto            base            = dynamic_cast<MasterMaterial_VK*>(get_base().get());
    const auto&           pipeline_layout = base->get_pipeline_layout(command_buffer->get_render_pass());
    const auto&           pipeline        = base->get_pipeline(command_buffer->get_render_pass());
    const VkCommandBuffer cmd             = **dynamic_cast<CommandBuffer_VK*>(command_buffer);

    if (!pipeline_layout)
    {
        LOG_ERROR("pipeline layout is null");
        return false;
    }

    if (!pipeline)
    {
        LOG_ERROR("pipeline is null");
        return false;
    }

    if (base->get_properties().line_width != 1.0f)
        vkCmdSetLineWidth(cmd, base->get_properties().line_width);

    auto& pass_data = descriptor_sets[command_buffer->get_render_pass()];

    if ((*pass_data)->is_dirty)
    {
        auto& desc_set = (*pass_data)->write_descriptor_sets;
        desc_set       = std::vector<VkWriteDescriptorSet>();

        for (const auto& buffer : write_buffers)
        {
            const auto* binding = find_binding(buffer.first, command_buffer->get_render_pass());
            if (!binding)
                continue;
            desc_set.emplace_back(VkWriteDescriptorSet{
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = (*pass_data)->descriptor_set,
                .dstBinding       = binding->binding,
                .dstArrayElement  = 0,
                .descriptorCount  = 1,
                .descriptorType   = MasterMaterial_VK::vk_descriptor_type(binding->descriptor_type),
                .pImageInfo       = nullptr,
                .pBufferInfo      = &dynamic_cast<Buffer_VK*>(buffer.second.get())->get_buffer_infos(),
                .pTexelBufferView = nullptr,
            });
        }

        for (const auto& texture : write_textures)
        {
            const auto* binding = find_binding(texture.first, command_buffer->get_render_pass());
            if (!binding)
                continue;
            desc_set.emplace_back(VkWriteDescriptorSet{
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = (*pass_data)->descriptor_set,
                .dstBinding       = binding->binding,
                .dstArrayElement  = 0,
                .descriptorCount  = 1,
                .descriptorType   = MasterMaterial_VK::vk_descriptor_type(binding->descriptor_type),
                .pImageInfo       = &dynamic_cast<Texture_VK*>(texture.second.get())->get_descriptor_image_infos(),
                .pBufferInfo      = nullptr,
                .pTexelBufferView = nullptr,
            });
        }
        for (const auto& sampler : write_samplers)
        {
            const auto* binding = find_binding(sampler.first, command_buffer->get_render_pass());
            if (!binding)
                continue;
            desc_set.emplace_back(VkWriteDescriptorSet{
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = (*pass_data)->descriptor_set,
                .dstBinding       = binding->binding,
                .dstArrayElement  = 0,
                .descriptorCount  = 1,
                .descriptorType   = MasterMaterial_VK::vk_descriptor_type(binding->descriptor_type),
                .pImageInfo       = &dynamic_cast<Sampler_VK*>(sampler.second.get())->get_descriptor_sampler_infos(),
                .pBufferInfo      = nullptr,
                .pTexelBufferView = nullptr,
            });
        }
        vkUpdateDescriptorSets(get_device(), static_cast<uint32_t>((*pass_data)->write_descriptor_sets.size()), (*pass_data)->write_descriptor_sets.data(), 0, nullptr);

        (*pass_data)->is_dirty = false;
    }
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->pipeline_layout, 0, 1, &(*pass_data)->descriptor_set, 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
    return true;
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
} // namespace gfx::vulkan