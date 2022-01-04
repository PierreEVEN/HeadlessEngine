#include "shader_builder/shader_module.h"

#include <cpputils/logger.hpp>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <internal.h>

namespace shader_builder
{
ShaderModule::ShaderModule(const std::string& shader_code, EShaderStage shader_stage)
{
    EShLanguage stage = EShLangVertex;
    switch (shader_stage)
    {
    case EShaderStage::Vertex:
        stage = EShLangVertex;
        break;
    case EShaderStage::Fragment:
        stage = EShLangFragment;
        break;
    }

    const char* const string[]    = {shader_code.c_str()};
    const int         lengths[]   = {static_cast<int>(shader_code.size())};
    const char* const names[]     = {"main_shader"};
    glslang::TShader  hlsl_shader(stage);

    hlsl_shader.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 0);
    hlsl_shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    hlsl_shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    hlsl_shader.setEntryPoint("main");
    hlsl_shader.setStringsWithLengthsAndNames(string, lengths, names, 1);

    constexpr unsigned debug_message_type = EShMsgDefault | EShMsgReadHlsl | EShMsgAST | EShMsgHlslLegalization | EShMsgBuiltinSymbolTable;

    std::string output;

    if (!hlsl_shader.parse(&get_resources(), 0, ENoProfile, false, false, static_cast<EShMessages>(debug_message_type)))
    {
        LOG_WARNING("parse debug : %s", hlsl_shader.getInfoDebugLog());
        LOG_ERROR("parse failed : %s", hlsl_shader.getInfoLog());
    }

    std::unique_ptr<glslang::TProgram> program = std::make_unique<glslang::TProgram>();

    program->addShader(&hlsl_shader);
    LOG_VALIDATE("added shader");
    if (!program->link(static_cast<EShMessages>(debug_message_type)))
    {
        LOG_ERROR("link failed : %s", program->getInfoLog());
    }

    int reflection_options = EShReflectionDefault;

    program->buildReflection(reflection_options);
    program->dumpReflection();

    std::vector<uint32_t> spirv;
    spv::SpvBuildLogger   logger;
    glslang::SpvOptions   spv_option;

    spv_option.generateDebugInfo = true;
    spv_option.disableOptimizer  = false;
    spv_option.optimizeSize      = false;
    spv_option.disassemble       = false;
    spv_option.validate          = true;
    GlslangToSpv(*program->getIntermediate(stage), spirv, &logger, &spv_option);

    LOG_INFO("compile finished : %s", logger.getAllMessages().c_str());

    LOG_VALIDATE("DONE");
}

bool ShaderModule::is_successful() const
{
    return compilation_result.is_success;
}

const ShaderBuildResult& ShaderModule::get_compilation_result() const
{
    return compilation_result;
}
} // namespace shader_builder
