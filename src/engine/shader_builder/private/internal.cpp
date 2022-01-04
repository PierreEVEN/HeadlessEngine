#include "internal.h"

namespace shader_builder
{

TBuiltInResource internal_resources;

TBuiltInResource init_resources()
{
    internal_resources.maxLights                                 = 32;
    internal_resources.maxClipPlanes                             = 6;
    internal_resources.maxTextureUnits                           = 32;
    internal_resources.maxTextureCoords                          = 32;
    internal_resources.maxVertexAttribs                          = 64;
    internal_resources.maxVertexUniformComponents                = 4096;
    internal_resources.maxVaryingFloats                          = 64;
    internal_resources.maxVertexTextureImageUnits                = 32;
    internal_resources.maxCombinedTextureImageUnits              = 80;
    internal_resources.maxTextureImageUnits                      = 32;
    internal_resources.maxFragmentUniformComponents              = 4096;
    internal_resources.maxDrawBuffers                            = 32;
    internal_resources.maxVertexUniformVectors                   = 128;
    internal_resources.maxVaryingVectors                         = 8;
    internal_resources.maxFragmentUniformVectors                 = 16;
    internal_resources.maxVertexOutputVectors                    = 16;
    internal_resources.maxFragmentInputVectors                   = 15;
    internal_resources.minProgramTexelOffset                     = -8;
    internal_resources.maxProgramTexelOffset                     = 7;
    internal_resources.maxClipDistances                          = 8;
    internal_resources.maxComputeWorkGroupCountX                 = 65535;
    internal_resources.maxComputeWorkGroupCountY                 = 65535;
    internal_resources.maxComputeWorkGroupCountZ                 = 65535;
    internal_resources.maxComputeWorkGroupSizeX                  = 1024;
    internal_resources.maxComputeWorkGroupSizeY                  = 1024;
    internal_resources.maxComputeWorkGroupSizeZ                  = 64;
    internal_resources.maxComputeUniformComponents               = 1024;
    internal_resources.maxComputeTextureImageUnits               = 16;
    internal_resources.maxComputeImageUniforms                   = 8;
    internal_resources.maxComputeAtomicCounters                  = 8;
    internal_resources.maxComputeAtomicCounterBuffers            = 1;
    internal_resources.maxVaryingComponents                      = 60;
    internal_resources.maxVertexOutputComponents                 = 64;
    internal_resources.maxGeometryInputComponents                = 64;
    internal_resources.maxGeometryOutputComponents               = 128;
    internal_resources.maxFragmentInputComponents                = 128;
    internal_resources.maxImageUnits                             = 8;
    internal_resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
    internal_resources.maxCombinedShaderOutputResources          = 8;
    internal_resources.maxImageSamples                           = 0;
    internal_resources.maxVertexImageUniforms                    = 0;
    internal_resources.maxTessControlImageUniforms               = 0;
    internal_resources.maxTessEvaluationImageUniforms            = 0;
    internal_resources.maxGeometryImageUniforms                  = 0;
    internal_resources.maxFragmentImageUniforms                  = 8;
    internal_resources.maxCombinedImageUniforms                  = 8;
    internal_resources.maxGeometryTextureImageUnits              = 16;
    internal_resources.maxGeometryOutputVertices                 = 256;
    internal_resources.maxGeometryTotalOutputComponents          = 1024;
    internal_resources.maxGeometryUniformComponents              = 1024;
    internal_resources.maxGeometryVaryingComponents              = 64;
    internal_resources.maxTessControlInputComponents             = 128;
    internal_resources.maxTessControlOutputComponents            = 128;
    internal_resources.maxTessControlTextureImageUnits           = 16;
    internal_resources.maxTessControlUniformComponents           = 1024;
    internal_resources.maxTessControlTotalOutputComponents       = 4096;
    internal_resources.maxTessEvaluationInputComponents          = 128;
    internal_resources.maxTessEvaluationOutputComponents         = 128;
    internal_resources.maxTessEvaluationTextureImageUnits        = 16;
    internal_resources.maxTessEvaluationUniformComponents        = 1024;
    internal_resources.maxTessPatchComponents                    = 120;
    internal_resources.maxPatchVertices                          = 32;
    internal_resources.maxTessGenLevel                           = 64;
    internal_resources.maxViewports                              = 16;
    internal_resources.maxVertexAtomicCounters                   = 0;
    internal_resources.maxTessControlAtomicCounters              = 0;
    internal_resources.maxTessEvaluationAtomicCounters           = 0;
    internal_resources.maxGeometryAtomicCounters                 = 0;
    internal_resources.maxFragmentAtomicCounters                 = 8;
    internal_resources.maxCombinedAtomicCounters                 = 8;
    internal_resources.maxAtomicCounterBindings                  = 1;
    internal_resources.maxVertexAtomicCounterBuffers             = 0;
    internal_resources.maxTessControlAtomicCounterBuffers        = 0;
    internal_resources.maxTessEvaluationAtomicCounterBuffers     = 0;
    internal_resources.maxGeometryAtomicCounterBuffers           = 0;
    internal_resources.maxFragmentAtomicCounterBuffers           = 1;
    internal_resources.maxCombinedAtomicCounterBuffers           = 1;
    internal_resources.maxAtomicCounterBufferSize                = 16384;
    internal_resources.maxTransformFeedbackBuffers               = 4;
    internal_resources.maxTransformFeedbackInterleavedComponents = 64;
    internal_resources.maxCullDistances                          = 8;
    internal_resources.maxCombinedClipAndCullDistances           = 8;
    internal_resources.maxSamples                                = 4;
    internal_resources.maxMeshOutputVerticesNV                   = 256;
    internal_resources.maxMeshOutputPrimitivesNV                 = 512;
    internal_resources.maxMeshWorkGroupSizeX_NV                  = 32;
    internal_resources.maxMeshWorkGroupSizeY_NV                  = 1;
    internal_resources.maxMeshWorkGroupSizeZ_NV                  = 1;
    internal_resources.maxTaskWorkGroupSizeX_NV                  = 32;
    internal_resources.maxTaskWorkGroupSizeY_NV                  = 1;
    internal_resources.maxTaskWorkGroupSizeZ_NV                  = 1;
    internal_resources.maxMeshViewCountNV                        = 4;

    internal_resources.limits.nonInductiveForLoops                 = 1;
    internal_resources.limits.whileLoops                           = 1;
    internal_resources.limits.doWhileLoops                         = 1;
    internal_resources.limits.generalUniformIndexing               = 1;
    internal_resources.limits.generalAttributeMatrixVectorIndexing = 1;
    internal_resources.limits.generalVaryingIndexing               = 1;
    internal_resources.limits.generalSamplerIndexing               = 1;
    internal_resources.limits.generalVariableIndexing              = 1;
    internal_resources.limits.generalConstantMatrixVectorIndexing  = 1;

    return internal_resources;
}
TBuiltInResource& get_resources()
{
    return internal_resources;
}
}