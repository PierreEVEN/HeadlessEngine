#include "gfx/materials/master_material.h"

#include "vulkan/vk_material.h"

namespace gfx
{
std::shared_ptr<MasterMaterial> MasterMaterial::create(const shader_builder::CompilationResult& compilation_results)
{
    print_compilation_results(compilation_results);
    const auto material = std::make_shared<vulkan::MasterMaterial_VK>();
    material->rebuild_material(compilation_results);
    return material;
}

std::shared_ptr<MasterMaterial> MasterMaterial::create(const std::filesystem::path& shader_path)
{
    return create(shader_builder::compile_shader(shader_path));
}

} // namespace gfx