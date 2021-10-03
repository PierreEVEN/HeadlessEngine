

#include "assets/asset_material.h"

#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "assets/asset_shader_buffer.h"
#include "engine_interface.h"
#include "assets/asset_texture.h"
#include "magic_enum/magic_enum.h"
#include "rendering/graphics.h"
#include "scene/node_camera.h"

template <typename BufferType_T>
static void add_layout_binding_t(std::vector<VkDescriptorSetLayoutBinding>& result_bindings, std::unordered_map<std::string, uint32_t>& binding_map, const std::vector<ShaderProperty>& shader_properties,
                                 const std::unordered_map<std::string, BufferType_T>& user_properties, VkShaderStageFlags shader_stage, VkDescriptorType descriptor_type,
                                 const ShaderProperty* optional_user_property = nullptr)
{
    std::unordered_map<std::string, ShaderProperty> shader_content;
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

AMaterial::AMaterial(const ShaderStageConfiguration& in_vertex_stage, const ShaderStageConfiguration& in_fragment_stage, const std::vector<std::string>& use_with_render_passes,
                     MaterialPipelineConfiguration pipeline_configuration)
    : vertex_stage(in_vertex_stage), fragment_stage(in_fragment_stage)
{
    if (use_with_render_passes.empty())
        LOG_FATAL("you need to specify at least one render pass to be used with");

    // @TODO reimplement push constant support
    if (vertex_stage.shader->get_push_constants() || fragment_stage.shader->get_push_constants())
        LOG_ERROR("missing push constant parameter");

    for (const auto& stage : use_with_render_passes)
    {
        if (!Graphics::get()->get_renderer()->get_render_pass_configuration(stage))
        {
            LOG_WARNING("material %s is designed to be used with render pass %s, but renderer doesn't have any render pass with this name", to_string().c_str(), stage.c_str());
            continue;
        }

        per_stage_pipeline[stage] = {};
        auto& pipeline = per_stage_pipeline.find(stage)->second;

        pipeline_configuration.vertex_module = vertex_stage.shader;
        pipeline_configuration.fragment_module = fragment_stage.shader;
        pipeline_configuration.renderer_stages = stage;
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

const MaterialPipeline& AMaterial::get_pipeline_class(const std::string& render_pass) const
{
    const auto found_pipeline = per_stage_pipeline.find(render_pass);
    if (found_pipeline == per_stage_pipeline.end())
        LOG_FATAL("trying to get pipeline class on render pass %s", render_pass.c_str());

    return found_pipeline->second;
}

MaterialPipelineBindings AMaterial::make_layout_bindings()
{
    MaterialPipelineBindings                  pipeline_bindings = {};
    // Vertex UBO
    add_layout_binding_t(pipeline_bindings.descriptor_bindings, pipeline_bindings.vertex_binding_map, vertex_stage.shader->get_uniform_buffers(), vertex_stage.uniform_buffers, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                         vertex_stage.shader->get_scene_data_buffer());
    // Fragment UBO
    add_layout_binding_t(pipeline_bindings.descriptor_bindings, pipeline_bindings.fragment_binding_map, fragment_stage.shader->get_uniform_buffers(), fragment_stage.uniform_buffers, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                         fragment_stage.shader->get_scene_data_buffer());
    // Vertex SSBO
    add_layout_binding_t(pipeline_bindings.descriptor_bindings, pipeline_bindings.vertex_binding_map, vertex_stage.shader->get_storage_buffers(), vertex_stage.storage_buffers, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                         vertex_stage.shader->get_model_matrix_buffer());
    // Fragment SSBO
    add_layout_binding_t(pipeline_bindings.descriptor_bindings, pipeline_bindings.fragment_binding_map, fragment_stage.shader->get_storage_buffers(), fragment_stage.storage_buffers, VK_SHADER_STAGE_FRAGMENT_BIT,
                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                         fragment_stage.shader->get_model_matrix_buffer());
    // Vertex samplers
    add_layout_binding_t(pipeline_bindings.descriptor_bindings, pipeline_bindings.vertex_binding_map, vertex_stage.shader->get_image_samplers(), vertex_stage.textures, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    // Fragment samplers
    add_layout_binding_t(pipeline_bindings.descriptor_bindings, pipeline_bindings.fragment_binding_map, fragment_stage.shader->get_image_samplers(), fragment_stage.textures, VK_SHADER_STAGE_FRAGMENT_BIT,
                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    return pipeline_bindings;
}

void AMaterial::update_descriptor_sets(const std::string& render_pass, NCamera* in_camera, uint32_t imageIndex)
{
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

    const auto& vertex_bindings = pipeline.get_pipeline_configuration().descriptor_bindings.vertex_binding_map;
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
}