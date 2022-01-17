#include "gfx/material_instance.h"

#include "vulkan/vk_material_instance.h"

namespace gfx
{

std::shared_ptr<MaterialInstance> MaterialInstance::create(const std::shared_ptr<MasterMaterial>& base)
{
#if GFX_USE_VULKAN
    return std::make_shared<vulkan::MaterialInstance_VK>(base);
#else
    static_assert(false, "backend not supported");
#endif
}
}
