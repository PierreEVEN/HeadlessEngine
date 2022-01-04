#include "shader_builder/shader_builder.h"

#include <cpputils/logger.hpp>
#include <glslang/Public/ShaderLang.h>
#include <internal.h>
#include <string>

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include "custom_includer.h"

std::string shader = "Texture2D textureColor : register(t1); \n\
SamplerState                   samplerColor : register(s1);\n\
Texture2D                      textureNormalMap : register(t2);\n\
SamplerState                   samplerNormalMap : register(s2);\n\
\n\
struct VSOutput\n\
{\n\
    [[vk::location(0)]] float3 Normal : NORMAL0;\n\
    [[vk::location(1)]] float2 UV : TEXCOORD0;\n\
    [[vk::location(2)]] float3 Color : COLOR0;\n\
    [[vk::location(3)]] float3 WorldPos : POSITION0;\n\
    [[vk::location(4)]] float3 Tangent : TEXCOORD1;\n\
};\n\
\n\
struct FSOutput\n\
{\n\
    float4 Position : SV_TARGET0;\n\
    float4 Normal : SV_TARGET1;\n\
    float4 Albedo : SV_TARGET2;\n\
};\n\
\n\
FSOutput main(VSOutput input)\n\
{\n\
    FSOutput output = (FSOutput)0;\n\
    output.Position = float4(input.WorldPos, 1.0);\n\
\n\
    // Calculate normal in tangent space\n\
    float3   N     = normalize(input.Normal);\n\
    float3   T     = normalize(input.Tangent);\n\
    float3   B     = cross(N, T);\n\
    float3x3 TBN   = float3x3(T, B, N);\n\
    float3   tnorm = mul(normalize(textureNormalMap.Sample(samplerNormalMap, input.UV).xyz * 2.0 - float3(1.0, 1.0, 1.0)), TBN);\n\
    output.Normal  = float4(tnorm, 1.0);\n\
\n\
    output.Albedo = textureColor.Sample(samplerColor, input.UV);\n\
    return output;\n\
}\n\
";

namespace shader_builder
{

std::unique_ptr<ShaderBuilder> singleton;

ShaderBuilder* get()
{
    if (!singleton)
        singleton = std::unique_ptr<ShaderBuilder>(new ShaderBuilder());
    return singleton.get();
}
void destroy()
{
    singleton = nullptr;
}

void test()
{
    LOG_WARNING("try compile");
    if (!glslang::InitializeProcess())
        LOG_FATAL("failed to initialize glslang")

    const char* const                                 string[]    = {shader.c_str()};
    const int                                         lengths[]   = {static_cast<int>(shader.size())};
    const char* const                                 names[]     = {"main_shader"};
    std::unique_ptr<glslang::TShader>                 hlsl_shader = std::make_unique<glslang::TShader>(EShLangFragment);
    std::unique_ptr<glslang::TShader::ForbidIncluder> includer    = std::make_unique<glslang::TShader::ForbidIncluder>();

    hlsl_shader->setEnvInput(glslang::EShSourceHlsl, EShLangFragment, glslang::EShClientVulkan, 0);
    hlsl_shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    hlsl_shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    hlsl_shader->setEntryPoint("main");
    hlsl_shader->setStringsWithLengthsAndNames(string, lengths, names, 1);

    constexpr unsigned debug_message_type = EShMsgDefault | EShMsgReadHlsl | EShMsgAST | EShMsgHlslLegalization | EShMsgBuiltinSymbolTable;

    std::string output;

    if (!hlsl_shader->parse(&get_resources(), 0, ENoProfile, false, false, static_cast<EShMessages>(debug_message_type)))
    {
        LOG_WARNING("parse debug : %s", hlsl_shader->getInfoDebugLog());
        LOG_ERROR("parse failed : %s", hlsl_shader->getInfoLog());
    }

    std::unique_ptr<glslang::TProgram> program = std::make_unique<glslang::TProgram>();

    program->addShader(hlsl_shader.get());
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
    GlslangToSpv(*program->getIntermediate(EShLangFragment), spirv, &logger, &spv_option);

    LOG_INFO("compile finished : %s", logger.getAllMessages().c_str());

    LOG_VALIDATE("DONE");
}

void ShaderBuilder::add_include_dir(const std::filesystem::path& dir)
{
    include_paths.insert(dir);
}

ShaderBuilder::~ShaderBuilder()
{
    delete includer;
    glslang::FinalizeProcess();
}

ShaderBuilder::ShaderBuilder()
{
    if (!glslang::InitializeProcess())
        LOG_FATAL("failed to initialize glslang");
    init_resources();
    includer = new CustomIncluder(this);
}

} // namespace shader_builder
