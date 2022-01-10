#pragma once

#include "shader_builder/compiler.h"

namespace shader_builder::glslang_backend
{

class GlslangCompiler : public Compiler
{
  public:
    GlslangCompiler(EShaderLanguage source_language) : Compiler(source_language)
    {
    }
    std::vector<uint32_t> build_to_spirv(std::string& shader_code, EShaderLanguage source_language, EShaderStage shader_stage) override;
};

} // namespace shader_builder::glslang