
#include "shader_builder/compiler.h"

#include "custom_includer.h"
#include "internal.h"
#include "shader_builder/parser.h"
#include "shader_builder/shader_builder.h"

#include <cpputils/logger.hpp>
#include <functional>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <spirv_reflect.h>

#include "types/magic_enum.h"

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
    current_offset += type.type_size;

    /*
    LOG_WARNING("%s : %d,  %s", variable->name, variable->location, variable->semantic);
    for (int i = 0; i < 32; ++i)
    {
        if (variable->decoration_flags & 1 << i)
        {
            LOG_DEBUG("semantic : %s", magic_enum::enum_name(static_cast<SpvReflectDecorationFlagBits>(1 << i)).data());
        }
    }
    */

    return Property{
        .name     = name_split.size() == 2 ? name_split[1] : name_split[0],
        .type     = type,
        .offset   = offset,
        .location = variable->location,
    };
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
    uint32_t offset = 0;
    result.input_size = 0;
    result.output_size = 0;
    for (uint32_t i = 0; i < shader_module.input_variable_count; ++i)
    {
        if (!(shader_module.input_variables[i]->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN))
        {
            auto property = reflect_property(shader_module.input_variables[i], offset);
            result.inputs.emplace_back(property);
            result.input_size += property.type.type_size;
        }
    }
    offset = 0;
    for (uint32_t i = 0; i < shader_module.output_variable_count; ++i)
    {
        if (!(shader_module.output_variables[i]->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN))
        {
            auto property = reflect_property(shader_module.output_variables[i], offset);
            result.outputs.emplace_back(property);
            result.output_size += property.type.type_size;
        }
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

StageResult build_shader(glslang::TShader& shader, EShLanguage stage)
{
    StageResult compilation_result;
    if (!shader.parse(&get_resources(), 0, false, EShMsgDefault, *get()->get_includer()))
    {
        compilation_result.status.add_error({
            .column        = -1,
            .line          = -1,
            .error_message = "parsing failed : " + std::string(shader.getInfoLog()),
        });
        return compilation_result;
    }

    glslang::TProgram program{};
    program.addShader(&shader);
    if (!program.link(EShMsgDefault))
    {
        compilation_result.status.add_error({
            .column        = -1,
            .line          = -1,
            .error_message = "link failed : " + std::string(program.getInfoLog()),
        });
        return compilation_result;
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spv_option;

    spv_option.generateDebugInfo = true;
    spv_option.disableOptimizer  = false;
    spv_option.optimizeSize      = false;
    spv_option.disassemble       = false;
    spv_option.validate          = true;
    GlslangToSpv(*program.getIntermediate(stage), compilation_result.spirv, &logger, &spv_option);

    compilation_result.reflection = build_reflection(compilation_result.spirv);
    return compilation_result;
}

CompilationResult compile_shader(const std::filesystem::path& file_path)
{
    CompilationResult compilation_result;

    const auto parsed_shader_file = parser::parse_shader(file_path);

    compilation_result.status = parsed_shader_file.status;
    if (!compilation_result.status)
        return compilation_result;

    compilation_result.properties = parsed_shader_file.shader_properties;

    for (const auto& pass : parsed_shader_file.passes)
    {
        std::vector<const char*> vertex_strings(pass.second.vertex_chunks.size());
        std::vector<int32_t>     vertex_lengths(pass.second.vertex_chunks.size());
        std::vector<const char*> vertex_names(pass.second.vertex_chunks.size());
        for (size_t i = 0; i < pass.second.vertex_chunks.size(); ++i)
        {
            vertex_strings[i] = pass.second.vertex_chunks[i].content.c_str();
            vertex_lengths[i] = static_cast<int32_t>(pass.second.vertex_chunks[i].content.size());
            vertex_names[i]   = pass.second.vertex_chunks[i].file.c_str();
        }
        glslang::TShader vertex_shader(EShLangVertex);
        vertex_shader.setEnvInput(glslang::EShSourceHlsl, EShLangVertex, glslang::EShClientVulkan, 0);
        vertex_shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
        vertex_shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        vertex_shader.setEntryPoint("main");
        vertex_shader.setStringsWithLengthsAndNames(vertex_strings.data(), vertex_lengths.data(), vertex_names.data(), static_cast<int>(pass.second.vertex_chunks.size()));
        compilation_result.passes[pass.first].vertex = build_shader(vertex_shader, EShLangVertex);

        std::vector<const char*> fragment_strings(pass.second.fragment_chunks.size());
        std::vector<int32_t>     fragment_lengths(pass.second.fragment_chunks.size());
        std::vector<const char*> fragment_names(pass.second.fragment_chunks.size());
        for (size_t i = 0; i < pass.second.fragment_chunks.size(); ++i)
        {
            fragment_strings[i] = pass.second.fragment_chunks[i].content.c_str();
            fragment_lengths[i] = static_cast<int32_t>(pass.second.fragment_chunks[i].content.size());
            fragment_names[i]   = pass.second.fragment_chunks[i].file.c_str();
        }
        glslang::TShader fragment_shader(EShLangFragment);
        fragment_shader.setEnvInput(glslang::EShSourceHlsl, EShLangFragment, glslang::EShClientVulkan, 0);
        fragment_shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
        fragment_shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        fragment_shader.setEntryPoint("main");
        fragment_shader.setStringsWithLengthsAndNames(fragment_strings.data(), fragment_lengths.data(), fragment_names.data(), static_cast<int>(pass.second.fragment_chunks.size()));
        compilation_result.passes[pass.first].fragment = build_shader(fragment_shader, EShLangFragment);

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