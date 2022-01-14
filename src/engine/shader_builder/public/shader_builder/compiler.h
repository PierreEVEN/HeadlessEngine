#pragma once
#include "shader_builder/shader_types.h"

#include <filesystem>

namespace shader_builder
{
struct ShaderBlock
{
    std::string text;
    std::string name;
};

struct InterstageData
{
    std::unordered_map<std::string, int> stage_outputs;
    int                                  binding_index = -1;
};

class Compiler
{
  public:
    static std::shared_ptr<Compiler> create(EShaderLanguage source_language);

    virtual std::vector<uint32_t> build_to_spirv(const std::vector<ShaderBlock>& shader_code, EShaderLanguage source_language, EShaderStage shader_stage, const InterstageData& input_stage_data,
                                                 InterstageData& output_stage_data) = 0;

  protected:
    Compiler([[maybe_unused]] EShaderLanguage source_language)
    {
    }
};

CompilationResult compile_shader(const std::filesystem::path& file_path);
bool              print_compilation_errors(const CompilationResult& results);
void              print_compilation_results(const CompilationResult& results);
} // namespace shader_builder
