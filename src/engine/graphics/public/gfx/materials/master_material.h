#pragma once

#include "gfx/render_pass_reference.h"
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

        for (const auto& pass : compilation_results.passes)
        {
            if (const auto pass_id = RenderPassID::get(pass.first))
            {
                vertex_reflection.init(pass_id)   = pass.second.vertex.reflection;
                fragment_reflection.init(pass_id) = pass.second.fragment.reflection;
            }
        }
    }

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

  protected:
    MasterMaterial() = default;

  private:
    shader_builder::ShaderProperties                 shader_properties;
    RenderPassData<shader_builder::ReflectionResult> vertex_reflection;
    RenderPassData<shader_builder::ReflectionResult> fragment_reflection;
};
} // namespace gfx
