#pragma once

#include <filesystem>

#include "shader_builder/shader_types.h"

namespace shader_builder::parser
{
ParserResult parse_shader(const std::filesystem::path& file_path);
} // namespace shader_builder::parser
