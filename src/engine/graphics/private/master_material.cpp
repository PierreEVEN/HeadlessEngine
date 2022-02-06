#include "gfx/master_material.h"

#include "vulkan/vk_master_material.h"

namespace gfx
{
std::shared_ptr<MasterMaterial> MasterMaterial::create(const shader_builder::CompilationResult& compilation_results, MaterialOptions options)
{
#if GFX_USE_VULKAN
    print_compilation_errors(compilation_results);
    //print_compilation_results(compilation_results);
    const auto material = std::make_shared<vulkan::MasterMaterial_VK>(options);
    material->rebuild_material(compilation_results);
    return material;
#else
    static_assert(false, "backend not supported");
#endif
}

std::shared_ptr<MasterMaterial> MasterMaterial::create(const std::filesystem::path& shader_path, MaterialOptions options)
{
    return create(shader_builder::compile_shader(shader_path), options);
}

void MasterMaterial::rebuild_material(const shader_builder::CompilationResult& compilation_results)
{
    shader_properties = compilation_results.properties;

    enabled_render_passes.clear();
    for (const auto& pass : compilation_results.passes)
    {
        if (RenderPassID::exists(pass.first))
        {
            const auto pass_id                = RenderPassID::get(pass.first);
            vertex_reflection.init(pass_id)   = pass.second.vertex.reflection;
            fragment_reflection.init(pass_id) = pass.second.fragment.reflection;
            enabled_render_passes.emplace_back(pass_id);
        }
    }
}
} // namespace gfx