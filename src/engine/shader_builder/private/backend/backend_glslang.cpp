#include "backend/backend_glslang.h"

#include "internal.h"
#include "shader_builder/shader_builder.h"

#include <glslang/SPIRV/GlslangToSpv.h>
#include "custom_includer.h"

#include <cpputils/logger.hpp>

namespace shader_builder::glslang_backend
{
std::vector<uint32_t> GlslangCompiler::build_to_spirv(const std::vector<ShaderBlock>& shader_code, EShaderLanguage source_language, EShaderStage shader_stage)
{
    std::vector<uint32_t> compilation_result;

    const EShLanguage stage = shader_stage == EShaderStage::Vertex ? EShLangVertex : EShLangFragment;

    std::vector<const char*> vertex_strings(shader_code.size());
    std::vector<int32_t>     vertex_lengths(shader_code.size());
    std::vector<const char*> vertex_names(shader_code.size());
    for (size_t i = 0; i < shader_code.size(); ++i)
    {
        vertex_strings[i] = shader_code[i].text.c_str();
        vertex_lengths[i] = static_cast<int32_t>(shader_code[i].text.size());
        vertex_names[i]   = shader_code[i].name.c_str();
    }

    glslang::TShader shader(stage);
    shader.setEnvInput(source_language == EShaderLanguage::GLSL ? glslang::EShSourceGlsl : glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 0);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    shader.setEntryPoint("main");
    shader.setStringsWithLengthsAndNames(vertex_strings.data(), vertex_lengths.data(), vertex_names.data(), static_cast<int>(shader_code.size()));

    if (!shader.parse(&get_resources(), 0, false, EShMsgDefault, *get()->get_includer()))
    {
        LOG_FATAL("failed to parse : \n%s", shader.getInfoLog());
        return compilation_result;
    }

    glslang::TProgram program{};
    program.addShader(&shader);
    if (!program.link(EShMsgDefault))
    {
        LOG_FATAL("failed to link : \n%s", shader.getInfoLog());
        return compilation_result;
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spv_option;

    spv_option.generateDebugInfo = true;
    spv_option.disableOptimizer  = false;
    spv_option.optimizeSize      = false;
    spv_option.disassemble       = false;
    spv_option.validate          = true;
    GlslangToSpv(*program.getIntermediate(stage), compilation_result, &logger, &spv_option);

    return compilation_result;


}
}
