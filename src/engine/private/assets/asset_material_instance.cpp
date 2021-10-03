

#include "assets/asset_material_instance.h"

#include "assets/asset_material.h"
#include "rendering/graphics.h"
#include "scene/node_camera.h"

AMaterialInstance::AMaterialInstance(const TAssetPtr<AMaterial>& in_base_material) : base_material(in_base_material)
{
    shader_properties = {};
    for (const auto& stage : base_material->get_shader_stages())
    {
        for (const auto& property : stage->get_shader_config().properties)
        {
            if (const auto found_property = stage->find_property_by_name(property.get_property_name()))
            {
                shader_properties.emplace_back(ShaderInstanceProperty{
                    .base_property = property,
                    .write_descriptor_set =
                        {
                            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .pNext            = nullptr,
                            .dstSet           = VK_NULL_HANDLE, // set on runtime
                            .dstBinding       = found_property->location,
                            .dstArrayElement  = 0,
                            .descriptorCount  = 1,
                            .descriptorType   = property.get_descriptor_type(),
                            .pImageInfo       = nullptr, // set on runtime
                            .pBufferInfo      = nullptr, // set on runtime
                            .pTexelBufferView = nullptr,
                        },
                });
            }
            else
                LOG_ERROR("cannot find property %s in stage %s", property.get_property_name().c_str(), stage.to_string().c_str());
        }
    }
}

static void add_write_descriptor_set(std::vector<VkWriteDescriptorSet>& result_sets, VkDescriptorType descriptor_type, uint32_t binding, const VkDescriptorSet& desc_set, VkDescriptorImageInfo* image,
                                     VkDescriptorBufferInfo* buffer)
{
    result_sets.emplace_back(VkWriteDescriptorSet{
        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = nullptr,
        .dstSet           = desc_set,
        .dstBinding       = binding,
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = descriptor_type,
        .pImageInfo       = image,
        .pBufferInfo      = buffer,
        .pTexelBufferView = nullptr,
    });
}

void AMaterialInstance::update_descriptor_sets(const std::string& render_pass, NCamera* in_camera, uint32_t imageIndex)
{
    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {};

    for (auto& property : shader_properties)
    {
        property.write_descriptor_set.dstSet      = base_material->get_descriptor_sets(render_pass)[imageIndex];
        property.write_descriptor_set.pBufferInfo = property.base_property.get_descriptor_buffer_info(imageIndex);
        property.write_descriptor_set.pImageInfo  = property.base_property.get_descriptor_image_info(imageIndex);
        write_descriptor_sets.emplace_back(property.write_descriptor_set);
    }

    vkUpdateDescriptorSets(Graphics::get()->get_logical_device(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
}

VkPipelineLayout AMaterialInstance::get_pipeline_layout(const std::string& render_pass) const
{
    return base_material->get_pipeline_layout(render_pass);
}

VkPipeline AMaterialInstance::get_pipeline(const std::string& render_pass) const
{
    return base_material->get_pipeline(render_pass);
}

const std::vector<VkDescriptorSet>& AMaterialInstance::get_descriptor_sets(const std::string& render_pass) const
{
    return base_material->get_descriptor_sets(render_pass);
}
