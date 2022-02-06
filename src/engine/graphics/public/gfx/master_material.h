#pragma once

#include "gfx/render_pass_reference.h"
#include <filesystem>
#include <shader_builder/compiler.h>

namespace gfx
{
struct MaterialOptions
{
    std::optional<std::vector<shader_builder::Property>> input_stage_override;
};

class MasterMaterial
{
  public:
    virtual ~MasterMaterial() = default;
    static std::shared_ptr<MasterMaterial> create(const shader_builder::CompilationResult& compilation_results, MaterialOptions options = {});
    static std::shared_ptr<MasterMaterial> create(const std::filesystem::path& shader_path, MaterialOptions options = {});

    virtual void rebuild_material(const shader_builder::CompilationResult& compilation_results);

    [[nodiscard]] const shader_builder::ShaderProperties& get_properties() const
    {
        return shader_properties;
    }

    [[nodiscard]] RenderPassData<shader_builder::ReflectionResult>& get_vertex_reflections()
    {
        return vertex_reflection;
    }
    [[nodiscard]] RenderPassData<shader_builder::ReflectionResult>& get_fragment_reflections()
    {
        return fragment_reflection;
    }

    [[nodiscard]] const shader_builder::ReflectionResult& get_vertex_reflection(const RenderPassID& render_pass_id) const
    {
        return vertex_reflection[render_pass_id];
    }
    [[nodiscard]] const shader_builder::ReflectionResult& get_fragment_reflection(const RenderPassID& render_pass_id) const
    {
        return fragment_reflection[render_pass_id];
    }

    [[nodiscard]] const std::vector<RenderPassID>& get_compatible_render_passes() const
    {
        return enabled_render_passes;
    }

  protected:
    MasterMaterial() = default;

  private:
    std::vector<RenderPassID>                        enabled_render_passes;
    shader_builder::ShaderProperties                 shader_properties;
    RenderPassData<shader_builder::ReflectionResult> vertex_reflection;
    RenderPassData<shader_builder::ReflectionResult> fragment_reflection;
};
} // namespace gfx
