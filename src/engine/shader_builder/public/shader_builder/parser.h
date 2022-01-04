#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "shader_builder/operation.h"

namespace shader_builder::parser
{
struct Chunk
{
    std::string file;
    uint32_t line_start;
    std::string content;
};

struct ShaderPass
{
    std::string pass_name;
    std::vector<Chunk> vertex_chunks;
    std::vector<Chunk> fragment_chunks;
};

struct Result
{
    std::unordered_map<std::string, ShaderPass>  passes;
    std::unordered_map<std::string, std::string> properties;
    std::unordered_map<std::string, std::string> default_values;
    OperationStatus                              status;
};

Result parse_shader(const std::filesystem::path& file_path);
}
