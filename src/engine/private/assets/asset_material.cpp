

#include "assets/asset_material.h"

#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "assets/asset_shader_buffer.h"
#include "assets/asset_texture.h"
#include "engine_interface.h"
#include "magic_enum/magic_enum.h"
#include "rendering/graphics.h"
#include "scene/node_camera.h"

template <typename BufferType_T>
static void add_layout_binding_t(std::vector<VkDescriptorSetLayoutBinding>& result_bindings, std::unordered_map<std::string, uint32_t>& binding_map, const std::vector<ShaderReflectProperty>& shader_properties,
                                 const std::unordered_map<std::string, BufferType_T>& user_properties, VkShaderStageFlags shader_stage, VkDescriptorType descriptor_type,
                                 const ShaderReflectProperty* optional_user_property = nullptr)
{
    std::unordered_map<std::string, ShaderReflectProperty> shader_content;
    // shader properties indexing
    for (const auto& property : shader_properties)
        shader_content[property.property_name] = property;

    std::vector<std::string> buffer_names;
    for (const auto& key : user_properties)
        buffer_names.emplace_back(key.first);
    if (optional_user_property) // register optional parameter
        buffer_names.emplace_back(optional_user_property->property_name);

    for (const auto& user_element : buffer_names)
    {
        if (const auto found_buffer = shader_content.find(user_element); found_buffer != shader_content.end())
        {
            result_bindings.emplace_back(VkDescriptorSetLayoutBinding{
                .binding            = found_buffer->second.location,
                .descriptorType     = descriptor_type,
                .descriptorCount    = 1,
                .stageFlags         = shader_stage,
                .pImmutableSamplers = nullptr,
            });

            binding_map[found_buffer->second.property_name] = found_buffer->second.location;

            shader_content.erase(user_element);
        }
        else
        {
            LOG_FATAL("given shader property %s doesn't exists in the current shader context", user_element.c_str());
        }
    }

    for (const auto& member_left : shader_content)
        LOG_WARNING("You forget to give binding for shader property %s", member_left.first.c_str());
    if (!shader_content.empty())
        LOG_FATAL("cannot compile shader. Please see previous warnings for more informations");
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

std::vector<ShaderUserProperty> AMaterial::get_shader_properties() const
{
    std::vector<ShaderUserProperty> properties{};
    for (const auto& stage : get_shader_stages())
    {
        for (const auto& property : stage->get_shader_config().properties)
            properties.emplace_back(property);
    }
    return properties;
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

MaterialPipelineBindings AMaterial::make_layout_bindings()
{
    MaterialPipelineBindings pipeline_bindings = {};

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
                pipeline_bindings.descriptor_bindings.emplace_back(VkDescriptorSetLayoutBinding{
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

void AMaterial::update_descriptor_sets(const std::string& render_pass, NCamera* in_camera, uint32_t imageIndex)
{
    /*
    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {};

    const auto& pipeline_class = per_stage_pipeline.find(render_pass);
    if (pipeline_class == per_stage_pipeline.end())
        LOG_FATAL("material %s was not configured to be used with render pass %s", to_string().c_str(), render_pass.c_str());

    const auto& pipeline = pipeline_class->second;

    if (!pipeline.get_pipeline_configuration().is_valid())
    {
        LOG_FATAL("pipeline configuration for material %s is not valid (stage %s)", to_string().c_str(), render_pass.c_str());
    }

    const auto descriptor_sets = pipeline.get_descriptor_sets()[imageIndex];

    const auto& vertex_bindings   = pipeline.get_pipeline_configuration().descriptor_bindings.vertex_binding_map;
    const auto& fragment_bindings = pipeline.get_pipeline_configuration().descriptor_bindings.fragment_binding_map;

    for (const auto& [key, asset] : vertex_stage.uniform_buffers)
    {
        if (!asset)
            LOG_FATAL("material %s : buffer %s is null", to_string().c_str(), asset.to_string().c_str());
        if (auto binding = vertex_bindings.find(key); binding != vertex_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding->second, descriptor_sets, nullptr, asset->get_descriptor_buffer_info(imageIndex));
        else
            LOG_ERROR("failed to find binding %s for vertex uniform buffer on material %s, pass %s", key.c_str(), key.c_str(), to_string().c_str(), render_pass.c_str());
    }

    for (const auto& [key, asset] : fragment_stage.uniform_buffers)
    {
        if (!asset)
            LOG_FATAL("material %s : buffer %s is null", to_string().c_str(), asset.to_string().c_str());
        if (auto binding = fragment_bindings.find(key); binding != fragment_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding->second, descriptor_sets, nullptr, asset->get_descriptor_buffer_info(imageIndex));
        else
            LOG_ERROR("failed to find binding %s for fragment uniform buffer on material %s, pass %s", key.c_str(), to_string().c_str(), render_pass.c_str());
    }

    for (const auto& [key, asset] : vertex_stage.storage_buffers)
    {
        if (!asset)
            LOG_FATAL("material %s : buffer %s is null", to_string().c_str(), asset.to_string().c_str());
        if (auto binding = vertex_bindings.find(key); binding != vertex_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding->second, descriptor_sets, nullptr, asset->get_descriptor_buffer_info(imageIndex));
        else
            LOG_ERROR("failed to find binding %s for vertex ssbo buffer on material %s, pass %s", key.c_str(), to_string().c_str(), render_pass.c_str());
    }

    for (const auto& [key, asset] : fragment_stage.storage_buffers)
    {
        if (!asset)
            LOG_FATAL("material %s : buffer %s is null", to_string().c_str(), asset.to_string().c_str());
        if (auto binding = fragment_bindings.find(key); binding != fragment_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding->second, descriptor_sets, nullptr, asset->get_descriptor_buffer_info(imageIndex));
        else
            LOG_ERROR("failed to find binding %s for fragment ssbo buffer on material %s, pass %s", key.c_str(), to_string().c_str(), render_pass.c_str());
    }

    for (const auto& [key, asset] : vertex_stage.textures)
    {
        auto* descriptor_image_infos = asset->get_descriptor_image_info(imageIndex);

        if (!descriptor_image_infos->sampler)
            LOG_FATAL("texture %s doesn't have any image sampler to be used by material %s", asset.to_string().c_str(), to_string().c_str());
        if (!asset)
            LOG_FATAL("material %s : texture %s is null", to_string().c_str(), asset.to_string().c_str());
        if (auto binding = vertex_bindings.find(key); binding != vertex_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding->second, descriptor_sets, descriptor_image_infos, nullptr);
        else
            LOG_ERROR("failed to find binding %s for vertex sampler on material %s, pass %s", key.c_str(), to_string().c_str(), render_pass.c_str());
    }

    for (const auto& [key, asset] : fragment_stage.textures)
    {
        auto* descriptor_image_infos = asset->get_descriptor_image_info(imageIndex);

        if (!descriptor_image_infos->sampler)
            LOG_FATAL("texture %s doesn't have any image sampler to be used by material %s", asset.to_string().c_str(), to_string().c_str());
        if (!asset)
            LOG_FATAL("material %s : texture %s is null", to_string().c_str(), asset.to_string().c_str());
        if (auto binding = fragment_bindings.find(key); binding != fragment_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding->second, descriptor_sets, descriptor_image_infos, nullptr);
        else
            LOG_ERROR("failed to find binding %s for fragment sampler on material %s, pass %s", key.c_str(), to_string().c_str(), render_pass.c_str());
    }

    // scene UBO
    if (const auto buffer = vertex_stage.shader->get_scene_data_buffer())
        add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, buffer->location, descriptor_sets, nullptr, in_camera->get_scene_uniform_buffer()->get_descriptor_buffer_info(imageIndex));

    if (const auto buffer = fragment_stage.shader->get_scene_data_buffer())
        add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, buffer->location, descriptor_sets, nullptr, in_camera->get_scene_uniform_buffer()->get_descriptor_buffer_info(imageIndex));

    // scene SSBO
    if (const auto buffer = vertex_stage.shader->get_model_matrix_buffer())
        add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer->location, descriptor_sets, nullptr, in_camera->get_model_ssbo()->get_descriptor_buffer_info(imageIndex));

    if (const auto buffer = fragment_stage.shader->get_model_matrix_buffer())
        add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer->location, descriptor_sets, nullptr, in_camera->get_model_ssbo()->get_descriptor_buffer_info(imageIndex));

    vkUpdateDescriptorSets(Graphics::get()->get_logical_device(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
    */
}