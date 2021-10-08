

#include "assets/asset_material.h"

#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "assets/asset_shader_buffer.h"
#include "assets/asset_texture.h"
#include "engine_interface.h"
#include "magic_enum/magic_enum.h"
#include "rendering/graphics.h"
#include "scene/node_camera.h"

bool MaterialInfos::is_valid() const
{
    return vertex_stage && fragment_stage && !renderer_passes.empty();
}

std::vector<TAssetPtr<AShader>> MaterialInfos::get_shader_stages() const
{
    std::vector<TAssetPtr<AShader>> stage_list;
    if (vertex_stage)
        stage_list.emplace_back(vertex_stage);
    if (tessellation_stage)
        stage_list.emplace_back(tessellation_stage);
    if (geometry_stage)
        stage_list.emplace_back(geometry_stage);
    if (fragment_stage)
        stage_list.emplace_back(fragment_stage);

    return stage_list;
}

AMaterial::AMaterial(const MaterialInfos& in_material_infos) : material_infos(in_material_infos)
{
    if (material_infos.renderer_passes.empty())
        LOG_FATAL("you need to specify at least one render pass to be used with");

    // @TODO reimplement push constant support

    auto layout_bindings = make_layout_bindings();

    for (const auto& pass : material_infos.renderer_passes)
    {
        if (!Graphics::get()->get_renderer()->get_render_pass_configuration(pass))
        {
            LOG_WARNING("material %s is designed to be used with render pass %s, but renderer doesn't have any render pass with this name", to_string().c_str(), pass.c_str());
            continue;
        }

        per_stage_pipeline[pass] = std::make_unique<MaterialPipeline>(material_infos.pipeline_infos, pass, layout_bindings, material_infos.get_shader_stages());
    }
}

MaterialPipeline* AMaterial::get_pipeline(const std::string& render_pass) const
{
    if (const auto found_pipeline = per_stage_pipeline.find(render_pass); found_pipeline != per_stage_pipeline.end())
    {
        return found_pipeline->second.get();
    }
    LOG_ERROR("material %s is not configured to be used with render pass %s", to_string().c_str(), render_pass.c_str());
    return nullptr;
}


std::vector<TAssetPtr<AShader>> AMaterial::get_shader_stages() const
{
    return material_infos.get_shader_stages();
}

const MaterialInfos& AMaterial::get_material_infos() const
{
    return material_infos;
}

static std::optional<ShaderReflectProperty> find_shader_property(const TAssetPtr<AShader>& shader, const std::string& property_name, const VkDescriptorType& descriptor_type)
{
    switch (descriptor_type)
    {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        for (const auto& property : shader->get_image_samplers())
            if (property.property_name == property_name)
                return property;
        break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        for (const auto& property : shader->get_uniform_buffers())
            if (property.property_name == property_name)
                return property;
        break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        for (const auto& property : shader->get_storage_buffers())
            if (property.property_name == property_name)
                return property;
        break;
    default:
        break;
    }

    return {};
}

std::vector<VkDescriptorSetLayoutBinding> AMaterial::make_layout_bindings() const
{
    std::vector<VkDescriptorSetLayoutBinding> pipeline_bindings = {};

    struct PropertySearchInfos
    {
        std::string      property_name;
        VkDescriptorType descriptor_type;
    };

    for (const auto& shader_stage : get_shader_stages())
    {
        std::vector<PropertySearchInfos> wanted_properties = {};

        for (const auto& property : shader_stage->get_shader_config().properties)
        {
            if (property.should_keep_in_buffer_structure())
                continue;
            wanted_properties.emplace_back(PropertySearchInfos{
                .property_name   = property.get_property_name(),
                .descriptor_type = property.get_descriptor_type(),
            });
        }

        if (shader_stage->get_shader_config().use_scene_object_buffer)
            wanted_properties.emplace_back(PropertySearchInfos{
                .property_name   = G_MODEL_MATRIX_BUFFER_NAME,
                .descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            });

        if (shader_stage->get_shader_config().use_view_data_buffer)
            wanted_properties.emplace_back(PropertySearchInfos{
                .property_name   = G_SCENE_DATA_BUFFER_NAME,
                .descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            });

        // retrieve user properties
        for (const auto& user_property : wanted_properties)
        {
            if (const auto found_property = find_shader_property(shader_stage, user_property.property_name, user_property.descriptor_type); found_property)
            {
                pipeline_bindings.emplace_back(VkDescriptorSetLayoutBinding{
                    .binding            = found_property->location,
                    .descriptorType     = user_property.descriptor_type,
                    .descriptorCount    = 1,
                    .stageFlags         = static_cast<VkShaderStageFlags>(shader_stage->get_shader_stage()),
                    .pImmutableSamplers = nullptr,
                });
            }
            else
                LOG_ERROR("failed to find property %s of type %s in shader stage %s", user_property.property_name.c_str(), magic_enum::enum_name(user_property.descriptor_type).data(), shader_stage->to_string().c_str());
        }
    }
    return pipeline_bindings;
}