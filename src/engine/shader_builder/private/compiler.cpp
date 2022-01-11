
#include "shader_builder/compiler.h"

#include "backend/backend_dxc.h"
#include "backend/backend_glslang.h"
#include "custom_includer.h"
#include "shader_builder/parser.h"
#include "types/magic_enum.h"
#include <cpputils/logger.hpp>
#include <functional>
#include <spirv-tools/libspirv.hpp>
#include <spirv_reflect.h>

namespace shader_builder
{

ETypeFormat get_type_format(SpvReflectTypeDescription* type, uint32_t& size)
{
    size = 0;
    if (type->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
    {
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
        {
            switch (type->traits.numeric.vector.component_count)
            {
            case 1:
                size = 4;
                return ETypeFormat::R32_SFLOAT;
            case 2:
                size = 8;
                return ETypeFormat::R32G32_SFLOAT;
            case 3:
                size = 12;
                return ETypeFormat::R32G32B32_SFLOAT;
            case 4:
                size = 16;
                return ETypeFormat::R32G32B32A32_SFLOAT;
            default:
                LOG_FATAL("unhandled type trait");
            }
        }
        else
        {
            size = 4;
            return ETypeFormat::R32_SFLOAT;
        }
    }
    if (type->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
    {
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
        {
            switch (type->traits.numeric.vector.component_count)
            {
            case 1:
                size = 4;
                return ETypeFormat::R32G32_SINT;
            case 2:
                size = 8;
                return ETypeFormat::R32G32_SINT;
            case 3:
                size = 12;
                return ETypeFormat::R32G32B32_SINT;
            case 4:
                size = 16;
                return ETypeFormat::R32G32B32A32_SINT;
            default:
                LOG_FATAL("unhandled type trait");
            }
        }
        else
        {
            size = 4;
            return ETypeFormat::R32G32_SINT;
        }
    }
    LOG_FATAL("unhandled type trait");
}
TypeInfo get_type(SpvReflectTypeDescription* type)
{
    if (!type)
    {
        LOG_WARNING("type is null");
        return {};
    }

    uint32_t    type_size;
    ETypeFormat format = get_type_format(type, type_size);

    return TypeInfo{
        .type_name = type->type_name ? type->type_name : "",
        .type_id   = reinterpret_cast<void*>(type),
        .type_size = type_size,
        .format    = format,
    };
}

Property reflect_property(SpvReflectInterfaceVariable* variable, uint32_t& current_offset)
{
    if (!variable)
    {
        LOG_WARNING("var is null");
        return {};
    }
    auto       name_split = stringutils::split(variable->name, {'.'});
    const auto type       = get_type(variable->type_description);
    uint32_t   offset     = current_offset;
    current_offset += static_cast<uint32_t>(type.type_size);

    return Property{
        .name     = name_split[name_split.size() - 1],
        .type     = type,
        .offset   = offset,
        .location = variable->location,
    };
}

EBindingType make_descriptor_type(SpvReflectDescriptorType spv_type)
{
    switch (spv_type)
    {
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
        return EBindingType::SAMPLER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return EBindingType::COMBINED_IMAGE_SAMPLER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return EBindingType::SAMPLED_IMAGE;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return EBindingType::STORAGE_IMAGE;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        return EBindingType::UNIFORM_TEXEL_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return EBindingType::STORAGE_TEXEL_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return EBindingType::UNIFORM_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return EBindingType::STORAGE_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        return EBindingType::UNIFORM_BUFFER_DYNAMIC;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        return EBindingType::STORAGE_BUFFER_DYNAMIC;
    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        return EBindingType::INPUT_ATTACHMENT;
    default:
        LOG_FATAL("unhandled descriptor type");
    }
}

ReflectionResult build_reflection(const std::vector<uint32_t>& spirv)
{
    ReflectionResult       result;
    SpvReflectShaderModule shader_module;
    if (spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &shader_module) != SPV_REFLECT_RESULT_SUCCESS)
    {
        result.status.add_error({
            .column        = -1,
            .line          = -1,
            .error_message = "reflection failed",
        });
        return result;
    }
    uint32_t offset    = 0;
    result.input_size  = 0;
    result.output_size = 0;
    for (uint32_t i = 0; i < shader_module.input_variable_count; ++i)
    {
        if (!(shader_module.input_variables[i]->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN))
        {
            auto property = reflect_property(shader_module.input_variables[i], offset);
            result.inputs.emplace_back(property);
            result.input_size += static_cast<uint32_t>(property.type.type_size);
        }
    }
    offset = 0;
    for (uint32_t i = 0; i < shader_module.output_variable_count; ++i)
    {
        if (!(shader_module.output_variables[i]->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN))
        {
            auto property = reflect_property(shader_module.output_variables[i], offset);
            result.outputs.emplace_back(property);
            result.output_size += static_cast<uint32_t>(property.type.type_size);
        }
    }

    for (uint32_t i = 0; i < shader_module.descriptor_binding_count; ++i)
    {
        const auto& binding = shader_module.descriptor_bindings[i];

        result.bindings.emplace_back(BindingDescriptor{
            .name            = std::string(binding.name).empty() ? binding.type_description->type_name : binding.name,
            .descriptor_type = make_descriptor_type(binding.descriptor_type),
            .binding         = binding.binding,
        });
    }

    return result;
}

OperationStatus check_pass_result(const PassResult& pass)
{
    OperationStatus result;

    if (!pass.status)
    {
        result.add_error({
            .column        = -1,
            .line          = -1,
            .error_message = "pass is not valid",
        });
        return result;
    }

    if (!pass.vertex.status)
    {
        result.add_error({
            .column        = -1,
            .line          = -1,
            .error_message = "vertex stage is not valid",
        });
        return result;
    }

    if (!pass.fragment.status)
    {
        result.add_error({
            .column        = -1,
            .line          = -1,
            .error_message = "fragment stage is not valid",
        });
        return result;
    }

    for (const auto& input : pass.fragment.reflection.inputs)
    {
        bool found_output = false;

        for (const auto& output : pass.vertex.reflection.outputs)
        {
            if (output.name == input.name)
            {
                found_output = output.location == input.location;
                break;
            }
        }
        if (!found_output)
        {
            result.add_error({
                .column        = -1,
                .line          = -1,
                .error_message = stringutils::format("missing output variable %s in vertex stage at location %d", input.name.c_str(), input.location),
            });
        }
    }
    if (!result)
        return result;

    return result;
}

std::shared_ptr<Compiler> Compiler::create(EShaderLanguage source_language)
{
    /* if (source_language == EShaderLanguage::HLSL)
        return std::make_shared<dxc_backend::DxcCompiler>(source_language);
    else*/
    return std::make_shared<glslang_backend::GlslangCompiler>(source_language);
}

CompilationResult compile_shader(const std::filesystem::path& file_path)
{
    CompilationResult compilation_result;

    const auto parsed_shader_file = parser::parse_shader(file_path);

    compilation_result.status = parsed_shader_file.status;
    if (!compilation_result.status)
        return compilation_result;

    compilation_result.properties = parsed_shader_file.shader_properties;

    auto compiler = Compiler::create(parsed_shader_file.shader_properties.shader_language);

    for (const auto& pass : parsed_shader_file.passes)
    {
        std::vector<ShaderBlock> vertex_blocks(pass.second.vertex_chunks.size());
        for (size_t i = 0; i < pass.second.vertex_chunks.size(); ++i)
        {
            vertex_blocks[i].text = pass.second.vertex_chunks[i].content;
            vertex_blocks[i].name = pass.second.vertex_chunks[i].file;
        }
        StageResult vertex_result;
        vertex_result.spirv                          = compiler->build_to_spirv(vertex_blocks, compilation_result.properties.shader_language, EShaderStage::Vertex);
        vertex_result.reflection                     = build_reflection(vertex_result.spirv);
        compilation_result.passes[pass.first].vertex = vertex_result;

        {
            const auto&    context = spvContextCreate(SPV_ENV_VULKAN_1_2);
            spv_text       text;
            spv_diagnostic diag;
            spvBinaryToText(context, vertex_result.spirv.data(), vertex_result.spirv.size(), 0, &text, &diag);
            spvContextDestroy(context);
            std::ofstream test_fs("output_vs.spv");
            test_fs << text->str;
            spvTextDestroy(text);
        }

        std::vector<ShaderBlock> fragment_block(pass.second.fragment_chunks.size());
        for (size_t i = 0; i < pass.second.fragment_chunks.size(); ++i)
        {
            fragment_block[i].text = pass.second.fragment_chunks[i].content;
            fragment_block[i].name = pass.second.fragment_chunks[i].file;
        }
        StageResult fragment_result;
        fragment_result.spirv                          = compiler->build_to_spirv(fragment_block, compilation_result.properties.shader_language, EShaderStage::Fragment);
        fragment_result.reflection                     = build_reflection(fragment_result.spirv);
        compilation_result.passes[pass.first].fragment = fragment_result;

        {
            const auto&    context = spvContextCreate(SPV_ENV_VULKAN_1_2);
            spv_text       text;
            spv_diagnostic diag;
            spvBinaryToText(context, fragment_result.spirv.data(), fragment_result.spirv.size(), 0, &text, &diag);
            spvContextDestroy(context);
            std::ofstream test_fs("output_fs.spv");
            test_fs << text->str;
            spvTextDestroy(text);
        }
        compilation_result.passes[pass.first].status = check_pass_result(compilation_result.passes[pass.first]);
    }

    return compilation_result;
}

bool print_status(const std::string& prefix, const OperationStatus& status)
{
    for (const auto& message : status.get_errors())
        LOG_ERROR("%s %d::%d \n%s", prefix.c_str(), message.line, message.column, message.error_message.c_str());
    return status;
}

bool print_compilation_results(const CompilationResult& results)
{
    if (!results.status)
        return print_status("shader compilation failed", results.status);

    for (const auto& pass : results.passes)
    {
        if (!pass.second.vertex.status)
            return print_status("shader pass [" + pass.first + "] error in vertex shader", pass.second.vertex.status);

        if (!pass.second.fragment.status)
            return print_status("shader pass [" + pass.first + "] error in fragment shader", pass.second.fragment.status);

        if (!pass.second.status)
            return print_status("shader pass [" + pass.first + "] error", pass.second.status);

        if (!pass.second.vertex.reflection.status)
            return print_status("shader pass [" + pass.first + "] error in vertex shader reflection", pass.second.vertex.reflection.status);

        if (!pass.second.fragment.reflection.status)
            return print_status("shader pass [" + pass.first + "] error in fragment shader reflection", pass.second.fragment.reflection.status);
    }
    return true;
}
} // namespace shader_builder