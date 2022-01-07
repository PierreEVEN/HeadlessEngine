#include "vk_material_instance.h"

#include "vk_material.h"

namespace gfx::vulkan
{
MaterialInstance_VK::MaterialInstance_VK(const std::shared_ptr<MasterMaterial>& base) : MaterialInstance(base)
{
    build_descriptor_sets();
}

void MaterialInstance_VK::build_descriptor_sets()
{

    const auto& property = get_base().get()->get_properties();

    for (const auto& base : property.)



}
} // namespace gfx::vulkan