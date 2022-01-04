#pragma once

#include <filesystem>
#include <glslang/Public/ShaderLang.h>

namespace shader_builder
{
class ShaderBuilder;

class CustomIncluder : public glslang::TShader::Includer
{
  public:
    CustomIncluder(ShaderBuilder* in_owner);

    void add_include_path(const std::filesystem::path& include_path);

    IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override;
    IncludeResult* includeLocal(const char* headerName, const char* includerName, [[maybe_unused]] size_t inclusionDepth) override;
    void releaseInclude(IncludeResult* include) override;

  private:
    IncludeResult* make_include_result(const std::filesystem::path& path);

    std::vector<std::filesystem::path> include_paths;
    ShaderBuilder*                     owner;
};

} // namespace shader_builder