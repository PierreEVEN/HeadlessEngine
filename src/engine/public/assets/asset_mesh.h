#pragma once

#include "asset_base.h"

class AMeshData;
class AMaterialInstance;

class Mesh : public AssetBase
{
  public:
    Mesh(const TAssetPtr<AMeshData>& in_mesh_data, const TAssetPtr<AMaterialInstance>& in_mesh_material) : mesh_data(in_mesh_data), mesh_material(in_mesh_material)
    {
    }

    const TAssetPtr<AMeshData>& get_mesh_data() const
    {
        return mesh_data;
    }

    const TAssetPtr<AMaterialInstance>& get_material() const
    {
        return mesh_material;
    }

  private:
    TAssetPtr<AMeshData> mesh_data;
    TAssetPtr<AMaterialInstance> mesh_material;
};
