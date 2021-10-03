

#include "assets/asset_material.h"

#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "assets/asset_shader_buffer.h"
#include "assets/asset_texture.h"
#include "engine_interface.h"
#include "magic_enum/magic_enum.h"
#include "rendering/gfx_context.h"
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

void add_write_descriptor_set(std::vector<VkWriteDescriptorSet>& result_sets, VkDescriptorType descriptor_type, uint32_t binding, const VkDescriptorSet& desc_set, VkDescriptorImageInfo* image,
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

AMaterial::AMaterial(const ShaderStageData& in_vertex_stage, const ShaderStageData& in_fragment_stage, const std::shared_ptr<PushConstant>& in_push_constant)
    : vertex_stage(in_vertex_stage), fragment_stage(in_fragment_stage), push_constant(in_push_constant)
{
    pipeline.set_vertex_module(vertex_stage.shader->get_shader_module_ptr());
    pipeline.set_fragment_module(fragment_stage.shader->get_shader_module_ptr());

    std::shared_ptr<VkPushConstantRange> push_constant_ranges;
    if (push_constant)
    {
        if (vertex_stage.shader->get_push_constants() || fragment_stage.shader->get_push_constants())
        {
            push_constant->stage_flags = vertex_stage.shader->get_push_constants() && fragment_stage.shader->get_push_constants() ? VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT
                                         : fragment_stage.shader->get_push_constants()                                            ? VK_SHADER_STAGE_FRAGMENT_BIT
                                                                                                                                  : VK_SHADER_STAGE_VERTEX_BIT;

            push_constant_ranges             = std::make_shared<VkPushConstantRange>();
            push_constant_ranges->stageFlags = push_constant->stage_flags;
            push_constant_ranges->offset     = 0;
            push_constant_ranges->size       = static_cast<uint32_t>(push_constant->get_size());
        }
        else
        {
            LOG_ERROR("specified push constant that is not available in current shaders");
        }
    }
    else if (vertex_stage.shader->get_push_constants() || fragment_stage.shader->get_push_constants())
    {
        LOG_ERROR("missing push constant parameter");
    }
    pipeline.set_push_constant_ranges(push_constant_ranges);
    pipeline.set_layout_bindings(make_layout_bindings());
    pipeline.rebuild();
}

std::vector<VkDescriptorSetLayoutBinding> AMaterial::make_layout_bindings()
{
    std::vector<VkDescriptorSetLayoutBinding> result_bindings;

    vertex_bindings.clear();
    fragment_bindings.clear();

    // Vertex UBO
    add_layout_binding_t(result_bindings, vertex_bindings, vertex_stage.shader->get_uniform_buffers(), vertex_stage.uniform_buffers, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                         vertex_stage.shader->get_scene_data_buffer());
    // Fragment UBO
    add_layout_binding_t(result_bindings, fragment_bindings, fragment_stage.shader->get_uniform_buffers(), fragment_stage.uniform_buffers, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                         fragment_stage.shader->get_scene_data_buffer());
    // Vertex SSBO
    add_layout_binding_t(result_bindings, vertex_bindings, vertex_stage.shader->get_storage_buffers(), vertex_stage.storage_buffers, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                         vertex_stage.shader->get_model_matrix_buffer());
    // Fragment SSBO
    add_layout_binding_t(result_bindings, fragment_bindings, fragment_stage.shader->get_storage_buffers(), fragment_stage.storage_buffers, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                         fragment_stage.shader->get_model_matrix_buffer());
    // Vertex samplers
    add_layout_binding_t(result_bindings, vertex_bindings, vertex_stage.shader->get_image_samplers(), vertex_stage.textures, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    // Fragment samplers
    add_layout_binding_t(result_bindings, fragment_bindings, fragment_stage.shader->get_image_samplers(), fragment_stage.textures, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    return result_bindings;
}

void AMaterial::update_descriptor_sets(NCamera* in_camera, uint32_t imageIndex)
{
    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {};

    const auto descriptor_sets = pipeline.get_descriptor_sets()[imageIndex];

    for (const auto& [key, asset] : vertex_stage.uniform_buffers)
    {
        if (auto binding = vertex_bindings.find(key); binding != vertex_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding->second, descriptor_sets, nullptr, asset->get_descriptor_buffer_info(imageIndex));
        else
            LOG_ERROR("failed to find binding for uniform buffer");
    }

    for (const auto& [key, asset] : fragment_stage.uniform_buffers)
    {
        if (auto binding = fragment_bindings.find(key); binding != fragment_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding->second, descriptor_sets, nullptr, asset->get_descriptor_buffer_info(imageIndex));
        else
            LOG_ERROR("failed to find binding for uniform buffer");
    }

    for (const auto& [key, asset] : vertex_stage.storage_buffers)
    {
        if (auto binding = vertex_bindings.find(key); binding != vertex_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding->second, descriptor_sets, nullptr, asset->get_descriptor_buffer_info(imageIndex));
        else
            LOG_ERROR("failed to find binding for ssbo buffer");
    }

    for (const auto& [key, asset] : fragment_stage.storage_buffers)
    {
        if (auto binding = fragment_bindings.find(key); binding != fragment_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding->second, descriptor_sets, nullptr, asset->get_descriptor_buffer_info(imageIndex));
        else
            LOG_ERROR("failed to find binding for ssbo buffer");
    }

    for (const auto& [key, asset] : vertex_stage.textures)
    {
        if (auto binding = vertex_bindings.find(key); binding != vertex_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding->second, descriptor_sets, asset->get_descriptor_image_info(imageIndex), nullptr);
        else
            LOG_ERROR("failed to find binding for sampler");
    }

    for (const auto& [key, asset] : fragment_stage.textures)
    {
        if (auto binding = fragment_bindings.find(key); binding != fragment_bindings.end())
            add_write_descriptor_set(write_descriptor_sets, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding->second, descriptor_sets, asset->get_descriptor_image_info(imageIndex), nullptr);
        else
            LOG_ERROR("failed to find binding for sampler");
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

    vkUpdateDescriptorSets(GfxContext::get()->logical_device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
}