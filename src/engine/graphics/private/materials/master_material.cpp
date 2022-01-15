#include "gfx/materials/master_material.h"

#include "vulkan/vk_material.h"

namespace gfx
{
std::shared_ptr<MasterMaterial> MasterMaterial::create(const shader_builder::CompilationResult& compilation_results, MaterialOptions options)
{
    print_compilation_errors(compilation_results);
    //print_compilation_results(compilation_results);
    const auto material = std::make_shared<vulkan::MasterMaterial_VK>(options);
    material->rebuild_material(compilation_results);
    return material;
}

std::shared_ptr<MasterMaterial> MasterMaterial::create(const std::filesystem::path& shader_path, MaterialOptions options)
{
    return create(shader_builder::compile_shader(shader_path), options);
}
} // namespace gfx