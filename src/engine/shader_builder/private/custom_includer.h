#pragma once

#include <glslang/Public/ShaderLang.h>

namespace shader_builder
{
class ShaderBuilder;

class CustomIncluder : public glslang::TShader::Includer
{
  public:
    CustomIncluder(ShaderBuilder* in_owner) : owner(in_owner)
    {
    }

    std::string get_include_data(const std::filesystem::path& include_path)
    {
        (void)include_path;
        return "TODO : FAIRE LE SYSTEME D'INCLUDE";
    }

  private:
    void releaseInclude(IncludeResult*) override
    {
    }
    ShaderBuilder* owner;
};

}