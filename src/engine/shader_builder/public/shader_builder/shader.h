#pragma once
#include "shader_reflection.h"

#include <filesystem>

namespace shader_builder
{
class Shader
{
  public:
    explicit Shader(const std::filesystem::path& source_path);

    explicit Shader(const std::string& source_data);

  private:
    std::unique_ptr<ShaderReflection> shader_reflector;
};

} // namespace shader_builder
