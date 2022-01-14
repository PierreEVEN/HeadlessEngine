#pragma once

#include "shader_builder/compiler.h"

#include <cpputils/logger.hpp>

namespace shader_builder::glslang_backend
{

class GlslangCompiler : public Compiler
{
  public:
    GlslangCompiler(EShaderLanguage source_language) : Compiler(source_language)
    {
    }
    std::vector<uint32_t> build_to_spirv(const std::vector<ShaderBlock>& shader_code, EShaderLanguage source_language, EShaderStage shader_stage, const InterstageData& input_stage_data,
                                         InterstageData& output_stage_data) override;
};

} // namespace shader_builder::glslang