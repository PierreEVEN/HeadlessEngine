

#include "assets/asset_material_instance.h"

#include "assets/asset_material.h"
#include "assets/asset_shader_buffer.h"
#include "assets/asset_texture.h"
#include "rendering/graphics.h"
#include "rendering/swapchain_config.h"
#include "scene/node_camera.h"

AMaterialInstance::AMaterialInstance(const TAssetPtr<AMaterialBase>& in_base_material) : base_material(in_base_material)
{
    textures = {};
    for (const auto& stage : base_material->get_shader_stages())
    {
        for (const auto& property : stage->get_shader_config().textures)
        {
            if (const auto found_property = stage->find_property_by_name(property.binding_name))
            {
                textures.emplace_back(TextureRuntimeProperty{
                    .base_property = property,
                    .write_descriptor_set =
                        {
                            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .pNext            = nullptr,
                            .dstSet           = VK_NULL_HANDLE, // set on runtime
                            .dstBinding       = found_property->location,
                            .dstArrayElement  = 0,
                            .descriptorCount  = 1,
                            .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .pImageInfo       = nullptr, // set on runtime
                            .pBufferInfo      = nullptr, // set on runtime
                            .pTexelBufferView = nullptr,
                        },
                });
            }
            else
                LOG_ERROR("cannot find texture %s in stage %s", property.binding_name.c_str(), stage.to_string().c_str());
        }

        if (stage->get_shader_config().use_view_data_buffer)
        {
            if (stage->get_shader_stage() == VK_SHADER_STAGE_VERTEX_BIT)
            {
                b_has_vertex_view_buffer = true;
                if (auto prop = stage->find_property_by_name(G_SCENE_DATA_BUFFER_NAME))
                    vertex_view_buffer_location = prop->location;
                else
                    LOG_ERROR("failed to find property %s", G_SCENE_DATA_BUFFER_NAME);
            }
            if (stage->get_shader_stage() == VK_SHADER_STAGE_FRAGMENT_BIT)
            {
                b_has_fragment_view_buffer = true;
                if (auto prop = stage->find_property_by_name(G_SCENE_DATA_BUFFER_NAME))
                    fragment_view_buffer_location = prop->location;
                else
                    LOG_ERROR("failed to find property %s", G_SCENE_DATA_BUFFER_NAME);
            }
        }

        if (stage->get_shader_config().use_scene_object_buffer)
        {
            if (stage->get_shader_stage() == VK_SHADER_STAGE_VERTEX_BIT)
            {
                b_has_vertex_transform_buffer = true;

                if (auto prop = stage->find_property_by_name(G_MODEL_MATRIX_BUFFER_NAME))
                    vertex_transform_buffer_location = prop->location;
                else
                    LOG_ERROR("failed to find property %s", G_MODEL_MATRIX_BUFFER_NAME);
            }
            if (stage->get_shader_stage() == VK_SHADER_STAGE_FRAGMENT_BIT)
            {
                b_has_fragment_transform_buffer = true;

                if (auto prop = stage->find_property_by_name(G_MODEL_MATRIX_BUFFER_NAME))
                    fragment_transform_buffer_location = prop->location;
                else
                    LOG_ERROR("failed to find property %s", G_MODEL_MATRIX_BUFFER_NAME);
            }
        }
    }

    for (const auto& pass : base_material->get_material_infos().renderer_passes)
    {

        /** Allocate descriptor set */
        const uint32_t               swapchain_image_count = Graphics::get()->get_swapchain_config()->get_image_count();
        std::vector<VkDescriptorSet> descriptor_set(swapchain_image_count);
        VkDescriptorSetAllocateInfo  alloc_info{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool     = VK_NULL_HANDLE,
            .descriptorSetCount = swapchain_image_count,
            .pSetLayouts        = base_material->get_pipeline(pass)->get_descriptor_sets_layouts(),
        };
        Graphics::get()->get_descriptor_pool()->alloc_memory(alloc_info);
        VK_ENSURE(vkAllocateDescriptorSets(Graphics::get()->get_logical_device(), &alloc_info, descriptor_set.data()), "Failed to allocate descriptor sets");

        std::vector<DescriptorSetsState> out_desc = {};

        for (const auto& desc : descriptor_set)
        {
            out_desc.emplace_back(DescriptorSetsState{.descriptor_set = desc});
        }

        descriptor_sets[pass] = out_desc;
    }
}

void AMaterialInstance::update_descriptor_sets(const std::string& render_pass, NCamera* in_camera, uint32_t imageIndex)
{
    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {};

    auto* descriptor_set_group = get_descriptor_sets(render_pass);


    const auto descriptor_sets                   = (*descriptor_set_group)[imageIndex].descriptor_set;

    if (b_has_vertex_view_buffer)
    {
        write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = descriptor_sets, // set on runtime
            .dstBinding       = vertex_view_buffer_location,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = in_camera->get_scene_uniform_buffer()->get_descriptor_buffer_info(imageIndex),
            .pTexelBufferView = nullptr,
        });
    }

    if (b_has_fragment_view_buffer)
    {
        write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = descriptor_sets, // set on runtime
            .dstBinding       = fragment_view_buffer_location,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = in_camera->get_scene_uniform_buffer()->get_descriptor_buffer_info(imageIndex),
            .pTexelBufferView = nullptr,
        });
    }

    if (b_has_vertex_transform_buffer)
    {
        write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = descriptor_sets, // set on runtime
            .dstBinding       = vertex_transform_buffer_location,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = in_camera->get_model_ssbo()->get_descriptor_buffer_info(imageIndex),
            .pTexelBufferView = nullptr,
        });
    }

    if (b_has_fragment_transform_buffer)
    {
        write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = descriptor_sets, // set on runtime
            .dstBinding       = fragment_transform_buffer_location,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = in_camera->get_model_ssbo()->get_descriptor_buffer_info(imageIndex),
            .pTexelBufferView = nullptr,
        });
    }

    for (auto& property : textures)
    {
        TAssetPtr<ATexture> texture = property.base_property.texture;
        if (!texture)
        {
            LOG_WARNING("texture used in material %s is NULL", to_string().c_str());
            texture = TAssetPtr<ATexture>("default_texture");
            if (!texture)
                LOG_FATAL("cannot find default texture");
        }
        VkWriteDescriptorSet descriptor = property.write_descriptor_set;
        descriptor.dstSet               = descriptor_sets;
        descriptor.pBufferInfo = nullptr, descriptor.pImageInfo = texture->get_descriptor_image_info(imageIndex);
        write_descriptor_sets.emplace_back(descriptor);
    }

    vkUpdateDescriptorSets(Graphics::get()->get_logical_device(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
}

std::vector<DescriptorSetsState>* AMaterialInstance::get_descriptor_sets(const std::string& render_pass)
{
    const auto descriptor_set_group = descriptor_sets.find(render_pass);

    if (descriptor_set_group == descriptor_sets.end())
    {
        LOG_ERROR("material instance %s cannot be used with render pass %s", to_string().c_str(), render_pass.c_str());
        LOG_DEBUG("allowed render pass are :");
        for (const auto& render_pass : descriptor_sets)
        {
            LOG_DEBUG("%s", render_pass.first);
        }
        return nullptr;
    }

    return &descriptor_set_group->second;
}