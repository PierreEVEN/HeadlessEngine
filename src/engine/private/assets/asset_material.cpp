

#include "assets/asset_material.h"

#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "assets/asset_shader_buffer.h"
#include "assets/asset_texture.h"
#include "engine_interface.h"
#include "magic_enum/magic_enum.h"
#include "rendering/graphics.h"
#include "scene/node_camera.h"

AMaterial::AMaterial(const TAssetPtr<AShader>& final_shader_stage, const std::vector<std::string>& use_with_render_passes, MaterialPipelineConfiguration pipeline_configuration,
                     std::optional<VertexInputConfig> vertex_input_override)
    : final_stage(final_shader_stage)
{
    if (use_with_render_passes.empty())
        LOG_FATAL("you need to specify at least one render pass to be used with");

    // @TODO reimplement push constant support

    for (const auto& stage : use_with_render_passes)
    {
        if (!Graphics::get()->get_renderer()->get_render_pass_configuration(stage))
        {
            LOG_WARNING("material %s is designed to be used with render pass %s, but renderer doesn't have any render pass with this name", to_string().c_str(), stage.c_str());
            continue;
        }

        per_stage_pipeline[stage] = {};
        auto& pipeline            = per_stage_pipeline.find(stage)->second;

        pipeline_configuration.shader_stages = get_shader_stages();
        pipeline_configuration.vertex_input =
            vertex_input_override ? vertex_input_override.value() :
            VertexInputConfig{ //@TODO handle vertex input
            },
        pipeline_configuration.renderer_stages     = stage;
        pipeline_configuration.descriptor_bindings = make_layout_bindings();
        pipeline.update_configuration(pipeline_configuration);
        pipeline.init_or_rebuild_pipeline();
    }
}

VkPipelineLayout AMaterial::get_pipeline_layout(const std::string& render_pass) const
{
    const auto layout = get_pipeline_class(render_pass).get_pipeline_layout();
    VK_CHECK(layout, stringutils::format("pipeline layout for pass %s is null", render_pass.c_str()).c_str());
    return layout;
}

VkPipeline AMaterial::get_pipeline(const std::string& render_pass) const
{
    const auto pipeline = get_pipeline_class(render_pass).get_pipeline();
    VK_CHECK(pipeline, stringutils::format("pipeline for pass %s is null", render_pass.c_str()).c_str());
    return pipeline;
}

const std::vector<VkDescriptorSet>& AMaterial::get_descriptor_sets(const std::string& render_pass) const
{
    const auto& desc_sets = get_pipeline_class(render_pass).get_descriptor_sets();
    if (desc_sets.empty())
        LOG_FATAL("pipeline for pass %s is null", render_pass.c_str());
    return desc_sets;
}

std::vector<TAssetPtr<AShader>> AMaterial::get_shader_stages() const
{
    std::vector<TAssetPtr<AShader>> shader_stages       = {};
    TAssetPtr<AShader>              current_final_stage = final_stage;
    while (current_final_stage)
    {
        shader_stages.emplace_back(current_final_stage);
        current_final_stage = current_final_stage->get_previous_shader_stage();
    }
    std::ranges::reverse(shader_stages);
    return shader_stages;
}

const MaterialPipeline& AMaterial::get_pipeline_class(const std::string& render_pass) const
{
    const auto found_pipeline = per_stage_pipeline.find(render_pass);
    if (found_pipeline == per_stage_pipeline.end())
        LOG_FATAL("trying to get pipeline class on render pass %s", render_pass.c_str());

    return found_pipeline->second;
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