

#include "assets/asset_shader.h"

#include "rendering/shaders/shaders_builder.h"

#include <magic_enum/magic_enum.h>
#include <spirv_cross/spirv_cross.hpp>

#define ENABLE_SHADER_LOGGING false

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

std::string ShaderReflectProperty::get_property_glsl_typename() const
{
    switch (property_type)
    {
    case spirv_cross::SPIRType::Unknown:
        return "unknown";
    case spirv_cross::SPIRType::Void:
        return "void";
    case spirv_cross::SPIRType::Boolean:
        return "bool";
    case spirv_cross::SPIRType::SByte:
        return "int8";
    case spirv_cross::SPIRType::UByte:
        return "uint8";
    case spirv_cross::SPIRType::Short:
        return "int16";
    case spirv_cross::SPIRType::UShort:
        return "uint16";
    case spirv_cross::SPIRType::Int:
        return "int";
    case spirv_cross::SPIRType::UInt:
        return "uint";
    case spirv_cross::SPIRType::Int64:
        return "int64";
    case spirv_cross::SPIRType::UInt64:
        return "uint64";
    case spirv_cross::SPIRType::AtomicCounter:
        return "unknown_atomic_counter";
    case spirv_cross::SPIRType::Half:
        return "unknown_half";
    case spirv_cross::SPIRType::Float:
        switch (vec_size)
        {
        case 1:
            return "float";
        case 2:
            return "vec2";
        case 3:
            return "vec3";
        case 4:
            return "vec4";
        case 5:
            return "unhandled_float_type";
        }
    case spirv_cross::SPIRType::Double:
        return "double";
    case spirv_cross::SPIRType::Struct:
        return "unknown_struct";
    case spirv_cross::SPIRType::Image:
        return "unknown_image";
    case spirv_cross::SPIRType::SampledImage:
        return "unknown_sampled_image";
    case spirv_cross::SPIRType::Sampler:
        return "sampler2D";
    case spirv_cross::SPIRType::AccelerationStructure:
        return "unknown_acceleration_structure";
    case spirv_cross::SPIRType::RayQuery:
        return "unknown_ray_query";
    default:
        return "unhandled_case";
    }
}

AShader::AShader(const std::filesystem::path& source_mesh_path, const ShaderInfos& in_shader_configuration, const TAssetPtr<AShader>& input_stage) : shader_configuration(in_shader_configuration)
{
    if (auto shader_data = read_shader_file(source_mesh_path); shader_data)
    {
        if (in_shader_configuration.shader_stage != VK_SHADER_STAGE_VERTEX_BIT && !input_stage)
            LOG_FATAL("You should alway specify a valid input stage for shader stages that are not vertex stages");

        shader_module = std::make_unique<ShaderModule>();
        shader_module->set_shader_stage(shader_configuration.shader_stage);
        const ShaderPreprocessor preprocessor(shader_data.value(), in_shader_configuration, input_stage,
                                              in_shader_configuration.vertex_inputs_override ? in_shader_configuration.vertex_inputs_override.value() : Vertex::get_attribute_descriptions());
        shader_module->set_plain_text(preprocessor.try_get_shader_code());
        const auto& bytecode = shader_module->get_bytecode();
        if (auto error = shader_module->get_error())
        {
            LOG_ERROR("Failed to compile shader %s : %s", to_string().c_str(), error->error_string.c_str());
            LOG_INFO("\n\n%s\n\n", preprocessor.get_debug_code(error->error_line, error->error_column).c_str());
            LOG_FATAL("failed to compile shader module %s", to_string().c_str());
        }
        else
            build_reflection_data(bytecode);
        LOG_INFO("successfully compiled shader %s", to_string().c_str());
    }
    else LOG_ERROR("failed to read shader file %s", source_mesh_path.string().c_str());
}

AShader::AShader(const std::vector<uint32_t>& shader_bytecode, const ShaderInfos& in_shader_configuration) : shader_configuration(in_shader_configuration)
{
    shader_module = std::make_unique<ShaderModule>();
    shader_module->set_shader_stage(shader_configuration.shader_stage);
    shader_module->set_bytecode(shader_bytecode);
    build_reflection_data(shader_module->get_bytecode());
    LOG_INFO("successfully compiled shader %s", to_string().c_str());
}

VkShaderModule AShader::get_shader_module() const
{
    return shader_module->get_shader_module();
}

const std::optional<ShaderReflectProperty>& AShader::get_push_constants() const
{
    return push_constants;
}

const std::vector<ShaderReflectProperty>& AShader::get_uniform_buffers() const
{
    return uniform_buffer;
}

const std::vector<ShaderReflectProperty>& AShader::get_image_samplers() const
{
    return image_samplers;
}

const std::vector<ShaderReflectProperty>& AShader::get_storage_buffers() const
{
    return storage_buffer;
}

const std::vector<ShaderReflectProperty>& AShader::get_stage_inputs() const
{
    return shader_inputs;
}

const std::vector<ShaderReflectProperty>& AShader::get_stage_outputs() const
{
    return shader_outputs;
}

std::vector<ShaderReflectProperty> AShader::get_all_properties() const
{
    std::vector<ShaderReflectProperty> properties{};
    for (const auto& property : uniform_buffer)
        properties.emplace_back(property);
    for (const auto& property : storage_buffer)
        properties.emplace_back(property);
    for (const auto& property : image_samplers)
        properties.emplace_back(property);
    for (const auto& property : shader_inputs)
        properties.emplace_back(property);
    for (const auto& property : shader_outputs)
        properties.emplace_back(property);

    return properties;
}

std::optional<ShaderReflectProperty> AShader::find_property_by_name(const std::string& property_name) const
{
    for (const auto& property : get_all_properties())
    {
        if (property_name == property.property_name)
            return property;
    }
    return {};
}

const VkShaderStageFlagBits& AShader::get_shader_stage() const
{
    return shader_configuration.shader_stage;
}

const ShaderInfos& AShader::get_shader_config() const
{
    return shader_configuration;
}

uint32_t AShader::get_last_binding_index() const
{
    return last_binding_index;
}

const ShaderReflectProperty* AShader::get_model_matrix_buffer() const
{
    for (auto& buffer : storage_buffer)
    {
        if (buffer.property_name == G_MODEL_MATRIX_BUFFER_NAME)
            return &buffer;
    }
    return nullptr;
}

const ShaderReflectProperty* AShader::get_scene_data_buffer() const
{
    for (auto& buffer : uniform_buffer)
    {
        if (buffer.property_name == G_SCENE_DATA_BUFFER_NAME)
            return &buffer;
    }
    return nullptr;
}

ShaderModule* AShader::get_shader_module_ptr() const
{
    return shader_module.get();
}

void AShader::build_reflection_data(const std::vector<uint32_t>& bytecode)
{
    const spirv_cross::Compiler compiler(bytecode);

    last_binding_index = 0;

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

            push_constants = ShaderReflectProperty{.property_name  = push_constant_buffer.name,
                                                   .property_type  = var_type.basetype,
                                                   .structure_size = compiler.get_declared_struct_size(var_type),
                                                   .location       = 0,
                                                   .vec_size       = var_type.vecsize,
                                                   .shader_stage   = shader_configuration.shader_stage};
            for (int i = 0; i < var_type.member_types.size(); ++i)
            {
                const auto member_type = compiler.get_type(var_type.member_types[i]);
                push_constants->structure_properties.emplace_back(ShaderReflectProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                                        .property_type  = member_type.basetype,
                                                                                        .structure_size = 0,
                                                                                        // compiler.get_declared_struct_size(member_type),
                                                                                        .location     = 0,
                                                                                        .vec_size     = member_type.vecsize,
                                                                                        .shader_stage = shader_configuration.shader_stage});
            }
        }
    }

    /**
     *  UNIFORM BUFFERS
     */

    for (const auto& buffer : compiler.get_shader_resources().uniform_buffers)
    {
        auto var_type = compiler.get_type(buffer.type_id);

        ShaderReflectProperty new_buffer = ShaderReflectProperty{.property_name  = buffer.name,
                                                                 .property_type  = var_type.basetype,
                                                                 .structure_size = compiler.get_declared_struct_size(var_type),
                                                                 .location       = compiler.get_decoration(buffer.id, spv::DecorationBinding),
                                                                 .vec_size       = var_type.vecsize,
                                                                 .shader_stage   = shader_configuration.shader_stage};

        if (new_buffer.location > last_binding_index)
            last_binding_index = new_buffer.location;

        for (int i = 0; i < var_type.member_types.size(); ++i)
        {
            const auto member_type = compiler.get_type(var_type.member_types[i]);
            new_buffer.structure_properties.emplace_back(ShaderReflectProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                               .property_type  = member_type.basetype,
                                                                               .structure_size = 0,
                                                                               // compiler.get_declared_struct_size(member_type),
                                                                               .location     = 0,
                                                                               .vec_size     = member_type.vecsize,
                                                                               .shader_stage = shader_configuration.shader_stage});
        }
        uniform_buffer.emplace_back(new_buffer);
    }

    /**
     * STORAGE BUFFER
     */

    for (const auto& buffer : compiler.get_shader_resources().storage_buffers)
    {
        auto var_type = compiler.get_type(buffer.type_id);

        ShaderReflectProperty new_buffer = ShaderReflectProperty{.property_name  = buffer.name,
                                                                 .property_type  = var_type.basetype,
                                                                 .structure_size = compiler.get_declared_struct_size(var_type),
                                                                 .location       = compiler.get_decoration(buffer.id, spv::DecorationBinding),
                                                                 .vec_size       = var_type.vecsize,
                                                                 .shader_stage   = shader_configuration.shader_stage};

        if (new_buffer.location > last_binding_index)
            last_binding_index = new_buffer.location;

        for (int i = 0; i < var_type.member_types.size(); ++i)
        {
            const auto member_type = compiler.get_type(var_type.member_types[i]);
            new_buffer.structure_properties.emplace_back(ShaderReflectProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                               .property_type  = member_type.basetype,
                                                                               .structure_size = 0,
                                                                               // compiler.get_declared_struct_size(member_type),
                                                                               .location     = 0,
                                                                               .vec_size     = member_type.vecsize,
                                                                               .shader_stage = shader_configuration.shader_stage});
        }
        storage_buffer.emplace_back(new_buffer);
    }

    /**
     * SAMPLED IMAGES
     */

    for (const auto& image : compiler.get_shader_resources().sampled_images)
    {
        auto       var_type = compiler.get_type(image.type_id);
        const auto binding  = compiler.get_decoration(image.id, spv::DecorationBinding);
        image_samplers.emplace_back(ShaderReflectProperty{
            .property_name  = image.name,
            .property_type  = var_type.basetype,
            .structure_size = 0,
            .location       = binding,
            .vec_size       = var_type.vecsize,
            .shader_stage   = shader_configuration.shader_stage,
        });

        if (binding > last_binding_index)
            last_binding_index = binding;
    }

    /**
     * SHADER INPUTS - OUTPUT
     */

    for (const auto& input : compiler.get_shader_resources().stage_inputs)
    {
        auto var_type = compiler.get_type(input.type_id);
        shader_inputs.emplace_back(ShaderReflectProperty{.property_name  = input.name,
                                                         .property_type  = var_type.basetype,
                                                         .structure_size = 0,
                                                         .location       = compiler.get_decoration(input.id, spv::DecorationLocation),
                                                         .vec_size       = var_type.vecsize,
                                                         .shader_stage   = shader_configuration.shader_stage});
    }

    for (const auto& output : compiler.get_shader_resources().stage_outputs)
    {
        auto var_type = compiler.get_type(output.type_id);
        shader_outputs.emplace_back(ShaderReflectProperty{.property_name  = output.name,
                                                          .property_type  = var_type.basetype,
                                                          .structure_size = 0,
                                                          .location       = compiler.get_decoration(output.id, spv::DecorationLocation),
                                                          .vec_size       = var_type.vecsize,
                                                          .shader_stage   = shader_configuration.shader_stage});
    }

#if ENABLE_SHADER_LOGGING
    std::string shader_log;

    shader_log += stringutils::format("interface variables : %d\n", compiler.get_active_interface_variables().size());
    for (const auto& var : compiler.get_active_interface_variables())
    {
        auto& type = compiler.get_type_from_variable(var);
        shader_log += stringutils::format("\t-- %s %s\n", magic_enum::enum_name(type.basetype).data(), compiler.get_remapped_declared_block_name(var).c_str());
    }

    if (!shader_inputs.empty())
    {
        shader_log += stringutils::format("shader input : %d\n", shader_inputs.size());
        for (const auto& image : shader_inputs)
        {
            shader_log += stringutils::format("\t-- %s%d %s binding=%d\n", magic_enum::enum_name(image.property_type).data(), image.vec_size, image.property_name.c_str(), image.location);
        }
    }
    if (!shader_outputs.empty())
    {
        shader_log += stringutils::format("shader outputs : %d\n", shader_outputs.size());
        for (const auto& image : shader_outputs)
        {
            shader_log += stringutils::format("\t-- %s%d %s binding=%d\n", magic_enum::enum_name(image.property_type).data(), image.vec_size, image.property_name.c_str(), image.location);
        }
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

    LOG_DEBUG("\nsuccessfully shader module \n##################### [ %s ] #####################\nentry point = '%s'\n%s", get_id().to_string().c_str(), entry_point.c_str(), shader_log.c_str());
#endif
}
