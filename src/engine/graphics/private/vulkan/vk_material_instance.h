#pragma once
#include "gfx/materials/material_instance.h"

namespace gfx::vulkan
{
class MaterialInstance_VK : public MaterialInstance
{
public:
    MaterialInstance_VK(const std::shared_ptr<MasterMaterial>& base) : MaterialInstance(base) {}
};
}
