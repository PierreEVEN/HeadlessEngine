#pragma once

#include "shader_builder/compiler.h"

#include <cpputils/logger.hpp>

namespace shader_builder::nazarashader_backend
{
class NazaraShaderCompiler : public Compiler
{
  public:
    NazaraShaderCompiler(EShaderLanguage source_language) : Compiler(source_language)
    {
        LOG_VALIDATE("using NazaraShader compiler");
    }

    std::vector<uint32_t> build_to_spirv(const std::vector<ShaderBlock>& shader_code, EShaderLanguage source_language, EShaderStage shader_stage) override;
};
} // namespace shader_builder::dxc_backend