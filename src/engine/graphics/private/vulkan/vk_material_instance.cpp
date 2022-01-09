#include "vk_material_instance.h"

#include "gfx/buffer.h"
#include "gfx/render_pass.h"
#include "vk_buffer.h"
#include "vk_device.h"
#include "vk_material.h"
#include "vk_render_pass.h"

namespace gfx::vulkan
{
MaterialInstance_VK::MaterialInstance_VK(const std::shared_ptr<MasterMaterial>& base) : MaterialInstance(base)
{
    build_descriptor_sets();
}

void MaterialInstance_VK::bind_buffer(const std::string& binding_name, Buffer* in_buffer)
{
    for (const auto& pass_reflection : get_base()->get_vertex_reflections())
    {
        for (const auto& binding : pass_reflection.bindings)
        {
            if (binding.name == binding_name)
            {
                VkWriteDescriptorSet{
                    .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext            = nullptr,
                    .dstSet           = VK_NULL_HANDLE, //@TODO
                    .dstBinding       = binding.binding,
                    .dstArrayElement  = 0,
                    .descriptorCount  = 1,
                    .descriptorType   = MasterMaterial_VK::vk_descriptor_type(binding.descriptor_type),
                    .pImageInfo       = nullptr,
                    .pBufferInfo      = &dynamic_cast<Buffer_VK*>(in_buffer)->get_buffer_infos(),
                    .pTexelBufferView = nullptr,
                };
            }
        }
    }
}

void MaterialInstance_VK::build_descriptor_sets()
{
    MasterMaterial_VK* vk_base = dynamic_cast<MasterMaterial_VK*>(get_base().get());

    for (auto it = get_base()->get_vertex_reflections().begin(); it != get_base()->get_vertex_reflections().end(); ++it)
    {
        auto desc_set_layouts = vk_base->get_descriptor_set_layout(it.id());
        for (uint8_t image_index = 0; image_index < desc_set_layouts.get_max_instance_count(); ++image_index)
        {
            VkDescriptorSetAllocateInfo descriptor_info{
                .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext              = nullptr,
                .descriptorPool     = VK_NULL_HANDLE,
                .descriptorSetCount = 1,
                .pSetLayouts        = &desc_set_layouts[image_index],
            };
            dynamic_cast<RenderPass_VK*>(RenderPass::find(it.id()))->get_descriptor_pool().alloc_memory(descriptor_info);
            auto desc_sets = descriptor_sets.init(it.id());
            vkAllocateDescriptorSets(get_device(), &descriptor_info, &desc_sets[image_index]);
        }
    }
}
} // namespace gfx::vulkan