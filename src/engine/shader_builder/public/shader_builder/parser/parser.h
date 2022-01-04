#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

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
};

Result parse_shader(const std::string& shader_file);
}
