#pragma once
#include "shader_reflection.h"

#include <memory>

namespace shader_builder
{
enum class EShaderStage
{
    Vertex,
    Fragment
};


struct ShaderBuildResult
{
    bool        is_success = false;
    std::string message_log;
    uint32_t    error_line;
    uint32_t    error_column;
};

class ShaderModule
{
  public:
    ShaderModule(const std::string& shader_code, EShaderStage shader_stage);

    [[nodiscard]] bool                     is_successful() const;
    [[nodiscard]] const ShaderBuildResult& get_compilation_result() const;

  private:
    ShaderBuildResult                 compilation_result;
    std::unique_ptr<ShaderReflection> shader_reflection_data;
};

} // namespace shader_builder