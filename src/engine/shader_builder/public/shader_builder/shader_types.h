#pragma once

#include "operation.h"

#include <unordered_map>
#include <types/type_format.h>

namespace shader_builder
{
struct TypeInfo
{
    std::string           type_name;
    void*                 type_id;
    size_t                type_size;
    ETypeFormat           format;
};

struct Property
{
    std::string         name;
    TypeInfo            type;
    uint32_t            offset;
    uint32_t            location;
};

struct Uniform
{
    std::string name;
    void*       type_id;
    int         size       = 0;
    int         num_member = 0;
};

struct ReflectionResult
{
    OperationStatus       status;
    std::vector<Uniform>  uniform;
    std::vector<Property> inputs;
    std::vector<Property> outputs;
    uint32_t              input_size;
    uint32_t              output_size;
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

enum class EShaderLangage
{
    HLSL,
    GLSL
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
    std::string    shader_version  = "1.0";
    EShaderLangage shader_language = EShaderLangage::HLSL;
    ECulling       culling         = ECulling::Back;
    EFrontFace     front_face      = EFrontFace::CounterClockwise;
    ETopology      topology        = ETopology::Triangles;
    EPolygonMode   polygon_mode    = EPolygonMode::Fill;
    EAlphaMode     alpha_mode      = EAlphaMode::Opaque;
    bool           depth_test      = true;
    float          line_width      = 1.0f;
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
    std::unordered_map<std::string, std::string> properties;
    std::unordered_map<std::string, std::string> default_values;
    ShaderProperties                             shader_properties;
    OperationStatus                              status;
};

} // namespace shader_builder