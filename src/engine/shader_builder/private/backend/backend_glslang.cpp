#include "backend/backend_glslang.h"

#include "internal.h"
#include "shader_builder/shader_builder.h"

#include <glslang/SPIRV/GlslangToSpv.h>

namespace shader_builder::glslang_backend
{
std::vector<uint32_t> GlslangCompiler::build_to_spirv(std::string& shader_code, EShaderLanguage source_language, EShaderStage shader_stage)
{
    std::vector<uint32_t> compilation_result;

    const EShLanguage stage = shader_stage == EShaderStage::Vertex ? EShLangVertex : EShLangFragment;


    glslang::TShader vertex_shader(stage);
    vertex_shader.setEnvInput(source_language == EShaderLanguage::GLSL ? glslang::EShSourceGlsl : glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 0);
    vertex_shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    vertex_shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    vertex_shader.setEntryPoint("main");
    vertex_shader.setStringsWithLengthsAndNames(vertex_strings.data(), vertex_lengths.data(), vertex_names.data(), static_cast<int>(pass.second.vertex_chunks.size()));

    if (!vertex_shader.parse(&get_resources(), 0, false, EShMsgDefault, *get()->get_includer()))
    {
        return compilation_result;
    }

    glslang::TProgram program{};
    program.addShader(&vertex_shader);
    if (!program.link(EShMsgDefault))
    {

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
