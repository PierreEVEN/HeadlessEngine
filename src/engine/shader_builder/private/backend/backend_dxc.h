#pragma once

#include "shader_builder/compiler.h"

namespace shader_builder::dxc_backend
{
class DxcCompiler : public Compiler
{
public:
    DxcCompiler(EShaderLanguage source_language) : Compiler(source_language)
    {
    }

    std::vector<uint32_t> build_to_spirv(std::string& shader_code) override;
};
}