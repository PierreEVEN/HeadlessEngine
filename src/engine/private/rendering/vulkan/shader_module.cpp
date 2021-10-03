

#include "rendering/vulkan/shader_module.h"

#include "cpputils/logger.hpp"
#include "rendering/graphics.h"
#include "rendering/vulkan/common.h"

#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/glslang_c_shader_types.h>

static TBuiltInResource glslang_default_resource = {

    .maxLights                                 = 32,
    .maxClipPlanes                             = 6,
    .maxTextureUnits                           = 32,
    .maxTextureCoords                          = 32,
    .maxVertexAttribs                          = 64,
    .maxVertexUniformComponents                = 4096,
    .maxVaryingFloats                          = 64,
    .maxVertexTextureImageUnits                = 32,
    .maxCombinedTextureImageUnits              = 80,
    .maxTextureImageUnits                      = 32,
    .maxFragmentUniformComponents              = 4096,
    .maxDrawBuffers                            = 32,
    .maxVertexUniformVectors                   = 128,
    .maxVaryingVectors                         = 8,
    .maxFragmentUniformVectors                 = 16,
    .maxVertexOutputVectors                    = 16,
    .maxFragmentInputVectors                   = 15,
    .minProgramTexelOffset                     = -8,
    .maxProgramTexelOffset                     = 7,
    .maxClipDistances                          = 8,
    .maxComputeWorkGroupCountX                 = 65535,
    .maxComputeWorkGroupCountY                 = 65535,
    .maxComputeWorkGroupCountZ                 = 65535,
    .maxComputeWorkGroupSizeX                  = 1024,
    .maxComputeWorkGroupSizeY                  = 1024,
    .maxComputeWorkGroupSizeZ                  = 64,
    .maxComputeUniformComponents               = 1024,
    .maxComputeTextureImageUnits               = 16,
    .maxComputeImageUniforms                   = 8,
    .maxComputeAtomicCounters                  = 8,
    .maxComputeAtomicCounterBuffers            = 1,
    .maxVaryingComponents                      = 60,
    .maxVertexOutputComponents                 = 64,
    .maxGeometryInputComponents                = 64,
    .maxGeometryOutputComponents               = 128,
    .maxFragmentInputComponents                = 128,
    .maxImageUnits                             = 8,
    .maxCombinedImageUnitsAndFragmentOutputs   = 8,
    .maxCombinedShaderOutputResources          = 8,
    .maxImageSamples                           = 0,
    .maxVertexImageUniforms                    = 0,
    .maxTessControlImageUniforms               = 0,
    .maxTessEvaluationImageUniforms            = 0,
    .maxGeometryImageUniforms                  = 0,
    .maxFragmentImageUniforms                  = 8,
    .maxCombinedImageUniforms                  = 8,
    .maxGeometryTextureImageUnits              = 16,
    .maxGeometryOutputVertices                 = 256,
    .maxGeometryTotalOutputComponents          = 1024,
    .maxGeometryUniformComponents              = 1024,
    .maxGeometryVaryingComponents              = 64,
    .maxTessControlInputComponents             = 128,
    .maxTessControlOutputComponents            = 128,
    .maxTessControlTextureImageUnits           = 16,
    .maxTessControlUniformComponents           = 1024,
    .maxTessControlTotalOutputComponents       = 4096,
    .maxTessEvaluationInputComponents          = 128,
    .maxTessEvaluationOutputComponents         = 128,
    .maxTessEvaluationTextureImageUnits        = 16,
    .maxTessEvaluationUniformComponents        = 1024,
    .maxTessPatchComponents                    = 120,
    .maxPatchVertices                          = 32,
    .maxTessGenLevel                           = 64,
    .maxViewports                              = 16,
    .maxVertexAtomicCounters                   = 0,
    .maxTessControlAtomicCounters              = 0,
    .maxTessEvaluationAtomicCounters           = 0,
    .maxGeometryAtomicCounters                 = 0,
    .maxFragmentAtomicCounters                 = 8,
    .maxCombinedAtomicCounters                 = 8,
    .maxAtomicCounterBindings                  = 1,
    .maxVertexAtomicCounterBuffers             = 0,
    .maxTessControlAtomicCounterBuffers        = 0,
    .maxTessEvaluationAtomicCounterBuffers     = 0,
    .maxGeometryAtomicCounterBuffers           = 0,
    .maxFragmentAtomicCounterBuffers           = 1,
    .maxCombinedAtomicCounterBuffers           = 1,
    .maxAtomicCounterBufferSize                = 16384,
    .maxTransformFeedbackBuffers               = 4,
    .maxTransformFeedbackInterleavedComponents = 64,
    .maxCullDistances                          = 8,
    .maxCombinedClipAndCullDistances           = 8,
    .maxSamples                                = 4,
    .maxMeshOutputVerticesNV                   = 256,
    .maxMeshOutputPrimitivesNV                 = 512,
    .maxMeshWorkGroupSizeX_NV                  = 32,
    .maxMeshWorkGroupSizeY_NV                  = 1,
    .maxMeshWorkGroupSizeZ_NV                  = 1,
    .maxTaskWorkGroupSizeX_NV                  = 32,
    .maxTaskWorkGroupSizeY_NV                  = 1,
    .maxTaskWorkGroupSizeZ_NV                  = 1,
    .maxMeshViewCountNV                        = 4,
    .limits{
        .nonInductiveForLoops                 = 1,
        .whileLoops                           = 1,
        .doWhileLoops                         = 1,
        .generalUniformIndexing               = 1,
        .generalAttributeMatrixVectorIndexing = 1,
        .generalVaryingIndexing               = 1,
        .generalSamplerIndexing               = 1,
        .generalVariableIndexing              = 1,
        .generalConstantMatrixVectorIndexing  = 1,
    }};

void ShaderModule::set_bytecode(const std::vector<uint32_t>& in_bytecode)
{
    access_lock.lock();
    bytecode = in_bytecode;
    mark_dirty();
    access_lock.unlock();
}

static ShaderModule::ShaderCompilationError get_compilation_error(glslang_shader_t* shader)
{
    auto message = glslang_shader_get_info_log(shader);

    uint32_t error_line   = 0;
    uint32_t error_column = 0;

    auto error_code = stringutils::split(message, {':'});
    if (error_code.size() >= 3)
    {
        error_column = std::stoi(error_code[1]);
        error_line   = std::stoi(error_code[2]);
    }

    return ShaderModule::ShaderCompilationError{
        .error_string = message,
        .error_line   = error_line,
        .error_column = error_column,
    };
}

void ShaderModule::set_plain_text(const std::string& in_shader_text)
{
    glslang_stage_t glslang_shader_stage = GLSLANG_STAGE_COUNT;
    switch (shader_stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:
        glslang_shader_stage = GLSLANG_STAGE_VERTEX;
        break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        glslang_shader_stage = GLSLANG_STAGE_FRAGMENT;
        break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
        glslang_shader_stage = GLSLANG_STAGE_GEOMETRY;
        break;
    default:
        LOG_FATAL("unhandled shader kind");
    }

    const glslang_input_t input = {
        .language                          = GLSLANG_SOURCE_GLSL,
        .stage                             = glslang_shader_stage,
        .client                            = GLSLANG_CLIENT_VULKAN,
        .client_version                    = GLSLANG_TARGET_VULKAN_1_2,
        .target_language                   = GLSLANG_TARGET_SPV,
        .target_language_version           = GLSLANG_TARGET_SPV_1_3,
        .code                              = in_shader_text.c_str(),
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
        return;
    }

    if (!glslang_shader_parse(shader, &input))
    {
        last_compilation_error = get_compilation_error(shader);
        return;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        last_compilation_error = get_compilation_error(shader);
        return;
    }

    glslang_program_SPIRV_generate(program, input.stage);

    if (glslang_program_SPIRV_get_messages(program))
    {
        LOG_WARNING("%s", glslang_program_SPIRV_get_messages(program));
    }

    glslang_shader_delete(shader);
    const auto program_data = std::vector(glslang_program_SPIRV_get_ptr(program), glslang_program_SPIRV_get_ptr(program) + glslang_program_SPIRV_get_size(program));

    glslang_program_delete(program);

    set_bytecode(program_data);
}

void ShaderModule::set_shader_stage(VkShaderStageFlagBits in_shader_stage)
{
    access_lock.lock();
    shader_stage = in_shader_stage;
    mark_dirty();
    access_lock.unlock();
}

const VkShaderModule& ShaderModule::get_shader_module()
{
    access_lock.lock();
    if (b_is_dirty)
    {
        destroy();
        create_shader_module();
        b_is_dirty = false;
    }
    access_lock.unlock();
    return module;
}

std::optional<ShaderModule::ShaderCompilationError> ShaderModule::get_error()
{
    auto error             = last_compilation_error;
    last_compilation_error = {};
    return error;
}

bool ShaderModule::is_valid() const
{
    if (bytecode.empty())
        return false;
    return true;
}

const std::vector<uint32_t>& ShaderModule::get_bytecode()
{
    access_lock.lock();
    if (b_is_dirty)
    {
        destroy();
        create_shader_module();
        b_is_dirty = false;
    }
    access_lock.unlock();
    return bytecode;
}

ShaderModule::~ShaderModule()
{
    access_lock.lock();
    destroy();
    access_lock.unlock();
}

void ShaderModule::create_shader_module()
{
    if (bytecode.empty())
        return;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytecode.size() * sizeof(uint32_t);
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(bytecode.data());
    if (VkResult res = vkCreateShaderModule(Graphics::get()->get_logical_device(), &createInfo, vulkan_common::allocation_callback, &module); !res == VK_SUCCESS)
    {
        LOG_ERROR("Failed to create shader module : %d", static_cast<uint32_t>(res));
        module = VK_NULL_HANDLE;
    }
}

void ShaderModule::destroy()
{
    if (module != VK_NULL_HANDLE)
        vkDestroyShaderModule(Graphics::get()->get_logical_device(), module, vulkan_common::allocation_callback);
    module = VK_NULL_HANDLE;
}

void ShaderModule::mark_dirty()
{
    b_is_dirty = true;
}
