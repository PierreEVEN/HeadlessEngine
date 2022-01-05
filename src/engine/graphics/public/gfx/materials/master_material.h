#pragma once

#include <filesystem>
#include <shader_builder/compiler.h>

namespace gfx
{
class MasterMaterial
{
  public:
    virtual ~MasterMaterial() = default;
    static std::shared_ptr<MasterMaterial> create(const shader_builder::CompilationResult& compilation_results);
    static std::shared_ptr<MasterMaterial> create(const std::filesystem::path& shader_path);

    virtual void rebuild_material(const shader_builder::CompilationResult& compilation_results)
    {
        shader_properties = compilation_results.properties;
    }

    [[nodiscard]] const shader_builder::ShaderProperties& get_properties() const
    {
        return shader_properties;
    }

  protected:
    MasterMaterial() = default;

  private:
    shader_builder::ShaderProperties shader_properties;
};
} // namespace gfx
