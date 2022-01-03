#include "shader_builder_test.h"
#include <cpputils/logger.hpp>
#include <string>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/glslang_c_shader_types.h>
#include <spirv_reflect.h>

#include <glslang/Public/ShaderLang.h>

std::string             shader = "Texture2D textureColor : register(t1); \n\
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
static TBuiltInResource InitResources()
{
    TBuiltInResource Resources;

    Resources.maxLights                                 = 32;
    Resources.maxClipPlanes                             = 6;
    Resources.maxTextureUnits                           = 32;
    Resources.maxTextureCoords                          = 32;
    Resources.maxVertexAttribs                          = 64;
    Resources.maxVertexUniformComponents                = 4096;
    Resources.maxVaryingFloats                          = 64;
    Resources.maxVertexTextureImageUnits                = 32;
    Resources.maxCombinedTextureImageUnits              = 80;
    Resources.maxTextureImageUnits                      = 32;
    Resources.maxFragmentUniformComponents              = 4096;
    Resources.maxDrawBuffers                            = 32;
    Resources.maxVertexUniformVectors                   = 128;
    Resources.maxVaryingVectors                         = 8;
    Resources.maxFragmentUniformVectors                 = 16;
    Resources.maxVertexOutputVectors                    = 16;
    Resources.maxFragmentInputVectors                   = 15;
    Resources.minProgramTexelOffset                     = -8;
    Resources.maxProgramTexelOffset                     = 7;
    Resources.maxClipDistances                          = 8;
    Resources.maxComputeWorkGroupCountX                 = 65535;
    Resources.maxComputeWorkGroupCountY                 = 65535;
    Resources.maxComputeWorkGroupCountZ                 = 65535;
    Resources.maxComputeWorkGroupSizeX                  = 1024;
    Resources.maxComputeWorkGroupSizeY                  = 1024;
    Resources.maxComputeWorkGroupSizeZ                  = 64;
    Resources.maxComputeUniformComponents               = 1024;
    Resources.maxComputeTextureImageUnits               = 16;
    Resources.maxComputeImageUniforms                   = 8;
    Resources.maxComputeAtomicCounters                  = 8;
    Resources.maxComputeAtomicCounterBuffers            = 1;
    Resources.maxVaryingComponents                      = 60;
    Resources.maxVertexOutputComponents                 = 64;
    Resources.maxGeometryInputComponents                = 64;
    Resources.maxGeometryOutputComponents               = 128;
    Resources.maxFragmentInputComponents                = 128;
    Resources.maxImageUnits                             = 8;
    Resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
    Resources.maxCombinedShaderOutputResources          = 8;
    Resources.maxImageSamples                           = 0;
    Resources.maxVertexImageUniforms                    = 0;
    Resources.maxTessControlImageUniforms               = 0;
    Resources.maxTessEvaluationImageUniforms            = 0;
    Resources.maxGeometryImageUniforms                  = 0;
    Resources.maxFragmentImageUniforms                  = 8;
    Resources.maxCombinedImageUniforms                  = 8;
    Resources.maxGeometryTextureImageUnits              = 16;
    Resources.maxGeometryOutputVertices                 = 256;
    Resources.maxGeometryTotalOutputComponents          = 1024;
    Resources.maxGeometryUniformComponents              = 1024;
    Resources.maxGeometryVaryingComponents              = 64;
    Resources.maxTessControlInputComponents             = 128;
    Resources.maxTessControlOutputComponents            = 128;
    Resources.maxTessControlTextureImageUnits           = 16;
    Resources.maxTessControlUniformComponents           = 1024;
    Resources.maxTessControlTotalOutputComponents       = 4096;
    Resources.maxTessEvaluationInputComponents          = 128;
    Resources.maxTessEvaluationOutputComponents         = 128;
    Resources.maxTessEvaluationTextureImageUnits        = 16;
    Resources.maxTessEvaluationUniformComponents        = 1024;
    Resources.maxTessPatchComponents                    = 120;
    Resources.maxPatchVertices                          = 32;
    Resources.maxTessGenLevel                           = 64;
    Resources.maxViewports                              = 16;
    Resources.maxVertexAtomicCounters                   = 0;
    Resources.maxTessControlAtomicCounters              = 0;
    Resources.maxTessEvaluationAtomicCounters           = 0;
    Resources.maxGeometryAtomicCounters                 = 0;
    Resources.maxFragmentAtomicCounters                 = 8;
    Resources.maxCombinedAtomicCounters                 = 8;
    Resources.maxAtomicCounterBindings                  = 1;
    Resources.maxVertexAtomicCounterBuffers             = 0;
    Resources.maxTessControlAtomicCounterBuffers        = 0;
    Resources.maxTessEvaluationAtomicCounterBuffers     = 0;
    Resources.maxGeometryAtomicCounterBuffers           = 0;
    Resources.maxFragmentAtomicCounterBuffers           = 1;
    Resources.maxCombinedAtomicCounterBuffers           = 1;
    Resources.maxAtomicCounterBufferSize                = 16384;
    Resources.maxTransformFeedbackBuffers               = 4;
    Resources.maxTransformFeedbackInterleavedComponents = 64;
    Resources.maxCullDistances                          = 8;
    Resources.maxCombinedClipAndCullDistances           = 8;
    Resources.maxSamples                                = 4;
    Resources.maxMeshOutputVerticesNV                   = 256;
    Resources.maxMeshOutputPrimitivesNV                 = 512;
    Resources.maxMeshWorkGroupSizeX_NV                  = 32;
    Resources.maxMeshWorkGroupSizeY_NV                  = 1;
    Resources.maxMeshWorkGroupSizeZ_NV                  = 1;
    Resources.maxTaskWorkGroupSizeX_NV                  = 32;
    Resources.maxTaskWorkGroupSizeY_NV                  = 1;
    Resources.maxTaskWorkGroupSizeZ_NV                  = 1;
    Resources.maxMeshViewCountNV                        = 4;

    Resources.limits.nonInductiveForLoops                 = 1;
    Resources.limits.whileLoops                           = 1;
    Resources.limits.doWhileLoops                         = 1;
    Resources.limits.generalUniformIndexing               = 1;
    Resources.limits.generalAttributeMatrixVectorIndexing = 1;
    Resources.limits.generalVaryingIndexing               = 1;
    Resources.limits.generalSamplerIndexing               = 1;
    Resources.limits.generalVariableIndexing              = 1;
    Resources.limits.generalConstantMatrixVectorIndexing  = 1;

    return Resources;
}

namespace shader_builder
{
void test()
{
    LOG_WARNING("try compile");

    const char* const                                 string[]    = {shader.c_str()};
    const int                                         lengths[]   = {static_cast<int>(shader.size())};
    const char* const                                 names[]     = {"main_shader"};
    std::unique_ptr<glslang::TShader>                 hlsl_shader = std::make_unique<glslang::TShader>(EShLangFragment);
    std::unique_ptr<glslang::TShader::ForbidIncluder> includer    = std::make_unique<glslang::TShader::ForbidIncluder>();
    TBuiltInResource                                  resources   = InitResources();

    hlsl_shader->setEnvInput(glslang::EShSourceHlsl, EShLangFragment, glslang::EShClientVulkan, 100);
    hlsl_shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    hlsl_shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    hlsl_shader->setEntryPoint("main");
    hlsl_shader->setStringsWithLengthsAndNames(string, lengths, names, 1);

    constexpr unsigned debug_message_type = EShMsgDefault | EShMsgReadHlsl | EShMsgAST | EShMsgHlslLegalization | EShMsgBuiltinSymbolTable;

    std::string output;
    if (!hlsl_shader->preprocess(&resources, 100, ENoProfile, false, false, static_cast<EShMessages>(debug_message_type), &output, *includer))
    {

        LOG_WARNING("preprocess debug : %s", hlsl_shader->getInfoDebugLog());
        LOG_ERROR("preprocess failed : %s", hlsl_shader->getInfoLog());
    }

    if (!hlsl_shader->parse(&resources, 100, ENoProfile, false, false, static_cast<EShMessages>(debug_message_type)))
    {
        LOG_WARNING("parse debug : %s", hlsl_shader->getInfoDebugLog());
        LOG_ERROR("parse failed : %s", hlsl_shader->getInfoLog());
    }

    // hlsl_shader->preprocess()
    /*
        const glslang_input_t input = {
        .language                          = GLSLANG_SOURCE_HLSL,
        .stage                             = GLSLANG_STAGE_FRAGMENT,
        .client                            = GLSLANG_CLIENT_VULKAN,
        .client_version                    = GLSLANG_TARGET_VULKAN_1_2,
        .target_language                   = GLSLANG_TARGET_SPV,
        .target_language_version           = GLSLANG_TARGET_SPV_1_3,
        .code                              = shader.c_str(),
        .default_version                   = 100,
        .default_profile                   = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = false,
        .forward_compatible                = false,
        .messages                          = GLSLANG_MSG_DEFAULT_BIT,
        .resource                          = reinterpret_cast<const glslang_resource_t*>(&glslang_default_resource),
    };

    glslang_shader_t* shader = glslang_shader_create(&input);

    if (!glslang_shader_preprocess(shader, &input))
    {
        last_compilation_error = get_compilation_error(shader);
        LOG_ERROR("1: %s", last_compilation_error->error_string.c_str());
        return;
    }

    if (!glslang_shader_parse(shader, &input))
    {
        last_compilation_error = get_compilation_error(shader);
        LOG_ERROR("2: %s", last_compilation_error->error_string.c_str());
        return;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        last_compilation_error = get_compilation_error(shader);
        LOG_ERROR("3: %s", last_compilation_error->error_string.c_str());
        return;
    }

    glslang_program_SPIRV_generate(program, input.stage);

    if (glslang_program_SPIRV_get_messages(program))
    {
        LOG_WARNING("4: %s", glslang_program_SPIRV_get_messages(program));
    }

    glslang_shader_delete(shader);
    const auto program_data = std::vector(glslang_program_SPIRV_get_ptr(program), glslang_program_SPIRV_get_ptr(program) + glslang_program_SPIRV_get_size(program));

    glslang_program_delete(program);

    LOG_VALIDATE("zfazfaz DONE");
    SpvReflectShaderModule module;
    SpvReflectResult       result = spvReflectCreateShaderModule(program_data.size() * sizeof(uint32_t), program_data.data(), &module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    uint32_t var_count = 0;
    result             = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    SpvReflectInterfaceVariable** input_vars = static_cast<SpvReflectInterfaceVariable**>(malloc(var_count * sizeof(SpvReflectInterfaceVariable*)));
    result                                   = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);

    const char* entry_point;
    spvReflectGetEntryPoint(&module, entry_point);

    LOG_WARNING("%s", entry_point);

    for (size_t i = 0; i < var_count; ++i)
    {
        LOG_WARNING("%s", input_vars[i]->name);
    }

    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    spvReflectDestroyShaderModule(&module);
    */
}
} // namespace shader_builder
