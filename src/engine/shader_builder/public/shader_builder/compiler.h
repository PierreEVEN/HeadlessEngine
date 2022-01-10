#pragma once
#include "shader_builder/shader_types.h"

#include <filesystem>

namespace shader_builder
{

class Compiler
{
public:
    static std::shared_ptr<Compiler> create(EShaderLanguage source_language);

	virtual std::vector<uint32_t> build_to_spirv(std::string& shader_code) = 0;

protected:
        Compiler(EShaderLanguage source_language);
};



CompilationResult compile_shader(const std::filesystem::path& file_path);
bool print_compilation_results(const CompilationResult& results);
} // namespace shader_builder
