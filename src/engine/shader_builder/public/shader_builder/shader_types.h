#pragma once

#include "operation.h"

#include <optional>
#include <types/type_format.h>
#include <unordered_map>

namespace shader_builder
{
struct TypeInfo
{
    std::string type_name;
    void*       type_id;
    size_t      type_size;
    ETypeFormat format;
};

struct Property
{
    std::string name;
    TypeInfo    type;
    uint32_t    offset   = 0;
    uint32_t    location = 0;
};

enum class EBindingType
{
    SAMPLER,
    COMBINED_IMAGE_SAMPLER,
    SAMPLED_IMAGE,
    STORAGE_IMAGE,
    UNIFORM_TEXEL_BUFFER,
    STORAGE_TEXEL_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    UNIFORM_BUFFER_DYNAMIC,
    STORAGE_BUFFER_DYNAMIC,
    INPUT_ATTACHMENT
};

struct BindingDescriptor
{
    std::string  name;
    EBindingType descriptor_type;
    uint32_t     binding;
};

struct PushConstant
{
    uint32_t    structure_size;
};

struct ReflectionResult
{
    OperationStatus                status;
    std::vector<BindingDescriptor> bindings;
    std::vector<Property>          inputs;
    std::vector<Property>          outputs;
    std::optional<PushConstant>    push_constant;
    uint32_t                       input_size;
    uint32_t                       output_size;
};

struct StageResult
{
    OperationStatus       status;
    std::vector<uint32_t> spirv;
    ReflectionResult      reflection;
};

struct PassResult
{
    OperationStatus status;
    StageResult     fragment;
    StageResult     vertex;
};

struct ParsedChunk
{
    OperationStatus result;
    std::string     file;
    uint32_t        line_start;
    std::string     content;
};

struct ShaderPass
{
    OperationStatus          result;
    std::string              pass_name;
    std::vector<ParsedChunk> vertex_chunks;
    std::vector<ParsedChunk> fragment_chunks;
};

enum class EShaderLanguage
{
    HLSL,
    GLSL
};

enum class EShaderStage
{
    Vertex,
    Fragment
};

enum class ECulling
{
    None,
    Front,
    Back,
    Both
};

enum class EFrontFace
{
    Clockwise,
    CounterClockwise,
};

enum class ETopology
{
    Points,
    Lines,
    Triangles,
};

enum class EPolygonMode
{
    Point,
    Line,
    Fill,
};

enum class EAlphaMode
{
    Opaque,
    Translucent,
    Additive
};

struct ShaderProperties
{
    std::string     shader_version  = "1.0";
    EShaderLanguage shader_language = EShaderLanguage::HLSL;
    ECulling        culling         = ECulling::Back;
    EFrontFace      front_face      = EFrontFace::CounterClockwise;
    ETopology       topology        = ETopology::Triangles;
    EPolygonMode    polygon_mode    = EPolygonMode::Fill;
    EAlphaMode      alpha_mode      = EAlphaMode::Opaque;
    bool            depth_test      = true;
    float           line_width      = 1.0f;
};

struct CompilationResult
{
    OperationStatus                             status;
    ShaderProperties                            properties;
    std::unordered_map<std::string, PassResult> passes;
};

struct ParserResult
{
    std::unordered_map<std::string, ShaderPass>  passes;
    std::unordered_map<std::string, std::string> default_values;
    ShaderProperties                             shader_properties;
    OperationStatus                              status;
};

} // namespace shader_builder