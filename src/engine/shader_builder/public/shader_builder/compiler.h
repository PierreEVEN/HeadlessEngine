#pragma once
#include "shader_builder/shader_types.h"

#include <filesystem>

namespace shader_builder
{
CompilationResult compile_shader(const std::filesystem::path& file_path);
bool print_compilation_results(const CompilationResult& results);
} // namespace shader_builder
