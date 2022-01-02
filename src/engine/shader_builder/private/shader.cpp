#include "shader.h"

#include "shader_parser.h"
#include "shader_reflection.h"

namespace shader_builder
{
Shader::Shader(const std::filesystem::path& source_path)
{
}

Shader::Shader(const std::string& source_data)
{
    ShaderCompiler compiler(source_data);
    shader_reflector = std::make_unique<ShaderReflector>(compiler);
}
}
