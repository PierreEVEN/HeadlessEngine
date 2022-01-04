
#include "shader_builder/compiler.h"

#include "custom_includer.h"
#include "internal.h"
#include "shader_builder/parser.h"
#include "shader_builder/shader_builder.h"

#include <cpputils/logger.hpp>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace shader_builder
{
CompilationResult compile_shader(const std::string& file_path)
{
    CompilationResult  compilation_result;
    constexpr unsigned debug_message_type = EShMsgDefault;
    constexpr int      reflection_options = EShReflectionDefault;

    const auto result = parser::parse_shader(file_path);

    compilation_result.status = result.status;
    if (!compilation_result.status)
        return compilation_result;

    LOG_INFO("build vertex shader");
    for (const auto& pass : result.passes)
    {
        std::vector<const char*> strings(pass.second.vertex_chunks.size());
        std::vector<int32_t>     lengths(pass.second.vertex_chunks.size());
        std::vector<const char*> names(pass.second.vertex_chunks.size());

        for (size_t i = 0; i < pass.second.vertex_chunks.size(); ++i)
        {
            strings[i] = pass.second.vertex_chunks[i].content.c_str();
            lengths[i] = static_cast<int32_t>(pass.second.vertex_chunks[i].content.size());
            names[i]   = pass.second.vertex_chunks[i].file.c_str();
        }

        glslang::TShader  hlsl_shader(EShLangVertex);
        glslang::TProgram program{};

        hlsl_shader.setEnvInput(glslang::EShSourceHlsl, EShLangVertex, glslang::EShClientVulkan, 0);
        hlsl_shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
        hlsl_shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        hlsl_shader.setEntryPoint("main");
        hlsl_shader.setStringsWithLengthsAndNames(strings.data(), lengths.data(), names.data(), static_cast<int>(pass.second.vertex_chunks.size()));

        if (!hlsl_shader.parse(&get_resources(), 0, false, static_cast<EShMessages>(debug_message_type), *get()->get_includer()))
        {
            LOG_ERROR("parse failed : %s", hlsl_shader.getInfoLog());
            return compilation_result;
        }

        program.addShader(&hlsl_shader);
        if (!program.link(static_cast<EShMessages>(debug_message_type)))
        {
            LOG_ERROR("link failed : %s", program.getInfoLog());
            return compilation_result;
        }

        program.buildReflection(reflection_options);
        program.dumpReflection();

        std::vector<uint32_t> spirv;
        spv::SpvBuildLogger   logger;
        glslang::SpvOptions   spv_option;

        spv_option.generateDebugInfo = true;
        spv_option.disableOptimizer  = false;
        spv_option.optimizeSize      = false;
        spv_option.disassemble       = false;
        spv_option.validate          = true;
        GlslangToSpv(*program.getIntermediate(EShLangVertex), spirv, &logger, &spv_option);
    }

    
    LOG_INFO("build fragment shader");
    for (const auto& pass : result.passes)
    {
        std::vector<const char*> strings(pass.second.fragment_chunks.size());
        std::vector<int32_t>     lengths(pass.second.fragment_chunks.size());
        std::vector<const char*> names(pass.second.fragment_chunks.size());

        for (size_t i = 0; i < pass.second.fragment_chunks.size(); ++i)
        {
            strings[i] = pass.second.fragment_chunks[i].content.c_str();
            lengths[i] = static_cast<int32_t>(pass.second.fragment_chunks[i].content.size());
            names[i]   = pass.second.fragment_chunks[i].file.c_str();
        }

        glslang::TShader  hlsl_shader(EShLangFragment);
        glslang::TProgram program{};

        hlsl_shader.setEnvInput(glslang::EShSourceHlsl, EShLangFragment, glslang::EShClientVulkan, 0);
        hlsl_shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
        hlsl_shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        hlsl_shader.setEntryPoint("main");
        hlsl_shader.setStringsWithLengthsAndNames(strings.data(), lengths.data(), names.data(), static_cast<int>(pass.second.vertex_chunks.size()));

        if (!hlsl_shader.parse(&get_resources(), 0, false, static_cast<EShMessages>(debug_message_type), *get()->get_includer()))
        {
            LOG_ERROR("parse failed : %s", hlsl_shader.getInfoLog());
            return compilation_result;
        }

        program.addShader(&hlsl_shader);
        if (!program.link(static_cast<EShMessages>(debug_message_type)))
        {
            LOG_ERROR("link failed : %s", program.getInfoLog());
            return compilation_result;
        }

        program.buildReflection(reflection_options);
        program.dumpReflection();

        std::vector<uint32_t> spirv;
        spv::SpvBuildLogger   logger;
        glslang::SpvOptions   spv_option;

        spv_option.generateDebugInfo = true;
        spv_option.disableOptimizer  = false;
        spv_option.optimizeSize      = false;
        spv_option.disassemble       = false;
        spv_option.validate          = true;
        GlslangToSpv(*program.getIntermediate(EShLangFragment), spirv, &logger, &spv_option);
    }

    return compilation_result;
}
} // namespace shader_builder