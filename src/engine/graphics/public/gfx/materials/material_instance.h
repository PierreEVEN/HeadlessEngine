#pragma once
#include "master_material.h"

#include <memory>

namespace gfx
{

class MaterialInstance
{
  public:
    std::shared_ptr<MaterialInstance> create(const std::shared_ptr<MasterMaterial>& base);

    [[nodiscard]] const std::shared_ptr<MasterMaterial>& get_base() const
    {
        return base_material;
    }

  protected:
    MaterialInstance(const std::shared_ptr<MasterMaterial>& base) : base_material(base)
    {
        
    }
  private:
    std::shared_ptr<MasterMaterial> base_material;
};
} // namespace gfx
