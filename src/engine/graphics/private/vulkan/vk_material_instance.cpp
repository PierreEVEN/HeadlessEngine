#include "vk_material_instance.h"

#include "vk_device.h"
#include "vk_material.h"

namespace gfx::vulkan
{
MaterialInstance_VK::MaterialInstance_VK(const std::shared_ptr<MasterMaterial>& base) : MaterialInstance(base)
{
    build_descriptor_sets();
}

void MaterialInstance_VK::build_descriptor_sets()
{
    MasterMaterial_VK* vk_base = dynamic_cast<MasterMaterial_VK*>(get_base().get());

    for (auto it = get_base()->get_vertex_reflections().begin(); it != get_base()->get_vertex_reflections().end(); ++it)
    {
        auto desc_set_layouts = vk_base->get_descriptor_set_layout(it.id());
        for (uint8_t image_index = 0; image_index < desc_set_layouts.get_max_instance_count(); ++image_index)
        {
            LOG_ERROR("todo");
            const VkDescriptorSetAllocateInfo descriptor_info{
                .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext              = nullptr,
                .descriptorPool     = 0,  //@TODO
                .descriptorSetCount = 1,
                .pSetLayouts        = &desc_set_layouts[image_index],
            };

            auto desc_sets = descriptor_sets.init(it.id());
            vkAllocateDescriptorSets(get_device(), &descriptor_info, &desc_sets[image_index]);
        }
    }
}
} // namespace gfx::vulkan