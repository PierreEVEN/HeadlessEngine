

#include "assets/asset_shader.h"

#include "StandAlone/ResourceLimits.h"
#include "spirv_cross.hpp"
#include <magic_enum/magic_enum.h>

#define ENABLE_SHADER_LOGGING true

std::optional<std::string> AShader::read_shader_file(const std::filesystem::path& source_path)
{
    std::optional<std::string> code;

    if (exists(source_path))
    {
        std::ifstream shader_file(source_path);

        std::string code_string;
        std::string line;
        while (std::getline(shader_file, line))
        {
            code_string += line + "\n";
        }

        shader_file.close();
        code = code_string;
    }

    return code;
}

AShader::AShader(const std::filesystem::path& source_mesh_path, EShaderStage in_shader_kind) : shader_stage(in_shader_kind)
{
    shader_module = std::make_shared<ShaderModule>();
    shader_module->set_shader_stage(shader_stage);
    if (auto shader_data = read_shader_file(source_mesh_path); shader_data)
    {
        shader_module->set_plain_text(*shader_data);

        build_reflection_data(shader_module->get_bytecode());
    }
    else
    {
        LOG_ERROR("failed to read shader file %s", source_mesh_path.string().c_str());
    }
}

VkShaderModule AShader::get_shader_module() const
{
    return shader_module->get_shader_module();
}

const std::optional<ShaderProperty>& AShader::get_push_constants() const
{
    return push_constants;
}

const std::vector<ShaderProperty>& AShader::get_uniform_buffers() const
{
    return uniform_buffer;
}

const std::vector<ShaderProperty>& AShader::get_image_samplers() const
{
    return image_samplers;
}

const std::vector<ShaderProperty>& AShader::get_storage_buffers() const
{
    return storage_buffer;
}

const ShaderProperty* AShader::get_model_matrix_buffer() const
{
    for (auto& buffer : storage_buffer)
    {
        if (buffer.property_name == G_MODEL_MATRIX_BUFFER_NAME)
            return &buffer;
    }
    return nullptr;
}

const ShaderProperty* AShader::get_scene_data_buffer() const
{
    for (auto& buffer : uniform_buffer)
    {
        if (buffer.property_name == G_SCENE_DATA_BUFFER_NAME)
            return &buffer;
    }
    return nullptr;
}

std::shared_ptr<ShaderModule> AShader::get_shader_module_ptr() const
{
    return shader_module;
}

void AShader::build_reflection_data(const std::vector<uint32_t>& bytecode)
{
    const spirv_cross::Compiler compiler(bytecode);

    /**
     * ENTRY POINTS
     */
    if (compiler.get_entry_points_and_stages().empty())
    {
        LOG_ERROR("shaders doesn't contains entry point");
    }
    else
    {
        entry_point = compiler.get_entry_points_and_stages()[0].name;
    }

    if (const auto push_constant_buffers = compiler.get_shader_resources().push_constant_buffers; !push_constant_buffers.empty())
    {
        if (push_constant_buffers.size() != 1)
        {
            LOG_ERROR("cannot handle more than one push constant buffer per shader");
        }
        else
        {
            const auto push_constant_buffer = push_constant_buffers[0];
            auto       var_type             = compiler.get_type(push_constant_buffer.type_id);

            push_constants = ShaderProperty{.property_name  = push_constant_buffer.name,
                                            .property_type  = var_type.basetype,
                                            .structure_size = compiler.get_declared_struct_size(var_type),
                                            .location       = 0,
                                            .vec_size       = var_type.vecsize,
                                            .shader_stage   = shader_stage};
            for (int i = 0; i < var_type.member_types.size(); ++i)
            {
                const auto member_type = compiler.get_type(var_type.member_types[i]);
                push_constants->structure_properties.emplace_back(ShaderProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                                 .property_type  = member_type.basetype,
                                                                                 .structure_size = 0,
                                                                                 // compiler.get_declared_struct_size(member_type),
                                                                                 .location     = 0,
                                                                                 .vec_size     = member_type.vecsize,
                                                                                 .shader_stage = shader_stage});
            }
        }
    }

    /**
     *  UNIFORM BUFFERS
     */

    for (const auto& buffer : compiler.get_shader_resources().uniform_buffers)
    {
        auto var_type = compiler.get_type(buffer.type_id);

        ShaderProperty new_buffer = ShaderProperty{.property_name  = buffer.name,
                                                   .property_type  = var_type.basetype,
                                                   .structure_size = compiler.get_declared_struct_size(var_type),
                                                   .location       = compiler.get_decoration(buffer.id, spv::DecorationBinding),
                                                   .vec_size       = var_type.vecsize,
                                                   .shader_stage   = shader_stage};
        for (int i = 0; i < var_type.member_types.size(); ++i)
        {
            const auto member_type = compiler.get_type(var_type.member_types[i]);
            new_buffer.structure_properties.emplace_back(ShaderProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                        .property_type  = member_type.basetype,
                                                                        .structure_size = 0,
                                                                        // compiler.get_declared_struct_size(member_type),
                                                                        .location     = 0,
                                                                        .vec_size     = member_type.vecsize,
                                                                        .shader_stage = shader_stage});
        }
        uniform_buffer.emplace_back(new_buffer);
    }

    /**
     * STORAGE BUFFER
     */

    for (const auto& buffer : compiler.get_shader_resources().storage_buffers)
    {
        auto var_type = compiler.get_type(buffer.type_id);

        ShaderProperty new_buffer = ShaderProperty{.property_name  = buffer.name,
                                                   .property_type  = var_type.basetype,
                                                   .structure_size = compiler.get_declared_struct_size(var_type),
                                                   .location       = compiler.get_decoration(buffer.id, spv::DecorationBinding),
                                                   .vec_size       = var_type.vecsize,
                                                   .shader_stage   = shader_stage};
        for (int i = 0; i < var_type.member_types.size(); ++i)
        {
            const auto member_type = compiler.get_type(var_type.member_types[i]);
            new_buffer.structure_properties.emplace_back(ShaderProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                        .property_type  = member_type.basetype,
                                                                        .structure_size = 0,
                                                                        // compiler.get_declared_struct_size(member_type),
                                                                        .location     = 0,
                                                                        .vec_size     = member_type.vecsize,
                                                                        .shader_stage = shader_stage});
        }
        storage_buffer.emplace_back(new_buffer);
    }

    /**
     * SAMPLED IMAGES
     */

    for (const auto& image : compiler.get_shader_resources().sampled_images)
    {
        auto var_type = compiler.get_type(image.type_id);
        image_samplers.emplace_back(ShaderProperty{.property_name  = image.name,
                                                   .property_type  = var_type.basetype,
                                                   .structure_size = 0,
                                                   .location       = compiler.get_decoration(image.id, spv::DecorationBinding),
                                                   .vec_size       = var_type.vecsize,
                                                   .shader_stage   = shader_stage});
    }

#if ENABLE_SHADER_LOGGING
    std::string shader_log;

    shader_log += stringutils::format("interface variables : %d\n", compiler.get_active_interface_variables().size());
    for (const auto& var : compiler.get_active_interface_variables())
    {
        auto& type = compiler.get_type_from_variable(var);
        shader_log += stringutils::format("\t-- %s %s\n", magic_enum::enum_name(type.basetype).data(), compiler.get_remapped_declared_block_name(var).c_str());
    }

    if (push_constants)
    {
        shader_log += stringutils::format("push constant : %s%d %s (%d bytes)\n", magic_enum::enum_name(push_constants->property_type).data(), push_constants->vec_size, push_constants->property_name.c_str(),
                                          push_constants->structure_size);
        for (const auto& struct_member : push_constants->structure_properties)
        {
            shader_log += stringutils::format("\t-- %s%d %s\n", magic_enum::enum_name(struct_member.property_type).data(), struct_member.vec_size, struct_member.property_name.c_str());
        }
    }

    if (!image_samplers.empty())
    {
        shader_log += stringutils::format("sampled images : %d\n", image_samplers.size());
        for (const auto& image : image_samplers)
        {
            shader_log += stringutils::format("\t-- %s%d %s binding=%d\n", magic_enum::enum_name(image.property_type).data(), image.vec_size, image.property_name.c_str(), image.location);
        }
    }

    if (!uniform_buffer.empty())
    {
        shader_log += stringutils::format("uniform buffers : %d\n", uniform_buffer.size());
        for (const auto& buffer : uniform_buffer)
        {
            shader_log +=
                stringutils::format("\t-- %s%d %s (%d bytes) binding=%d\n", magic_enum::enum_name(buffer.property_type).data(), buffer.vec_size, buffer.property_name.c_str(), buffer.structure_size, buffer.location);
            for (const auto& struct_member : buffer.structure_properties)
            {
                shader_log += stringutils::format("\t---- %s%d %s\n", magic_enum::enum_name(struct_member.property_type).data(), struct_member.vec_size, struct_member.property_name.c_str());
            }
        }
    }

    if (!storage_buffer.empty())
    {
        shader_log += stringutils::format("storage buffers : %d\n", storage_buffer.size());
        for (const auto& buffer : storage_buffer)
        {
            shader_log +=
                stringutils::format("\t-- %s%d %s (%d bytes) binding=%d\n", magic_enum::enum_name(buffer.property_type).data(), buffer.vec_size, buffer.property_name.c_str(), buffer.structure_size, buffer.location);
            for (const auto& struct_member : buffer.structure_properties)
            {
                shader_log += stringutils::format("\t---- %s%d %s\n", magic_enum::enum_name(struct_member.property_type).data(), struct_member.vec_size, struct_member.property_name.c_str());
            }
        }
    }

    LOG_DEBUG("\ncompiling shader module [%s] : entry point = '%s'\n%s", get_id().to_string().c_str(), entry_point.c_str(), shader_log.c_str());
#endif
}
