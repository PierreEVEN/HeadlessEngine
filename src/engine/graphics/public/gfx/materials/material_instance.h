#pragma once
#include "master_material.h"

#include <memory>

namespace gfx
{

class MaterialInstance
{
  public:
    static std::shared_ptr<MaterialInstance> create(const std::shared_ptr<MasterMaterial>& base);

    [[nodiscard]] const std::shared_ptr<MasterMaterial>& get_base() const
    {
        return base_material;
    }

    [[nodiscard]] const std::vector<RenderPassID>& get_compatible_render_passes() const
    {
        return base_material->get_compatible_render_passes();
    }

  protected:
    MaterialInstance(const std::shared_ptr<MasterMaterial>& base) : base_material(base)
    {
        
    }
  private:
    std::shared_ptr<MasterMaterial> base_material;
};
} // namespace gfx
