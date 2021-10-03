

#include "assets/asset_material_instance.h"

#include "assets/asset_material.h"

AMaterialInstance::AMaterialInstance(const TAssetPtr<AMaterial>& in_base_material) : base_material(in_base_material)
{
    shader_properties = base_material->get_shader_properties();
}
