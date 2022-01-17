#pragma once
#include "assets/asset_base.h"
#include "assets/asset_mesh_data.h"
#include "assets/asset_ptr.h"

namespace primitive
{
template <typename Primitive_T> TAssetPtr<AMeshData> create_primitive(const AssetId& asset_id)
{
    Primitive_T new_primitive;
    auto        asset = AssetManager::get()->create<AMeshData>(asset_id, new_primitive.get_vertices(), new_primitive.get_indices());
    return asset;
}

class Primitive
{
  public:
    virtual                                     ~Primitive() = default;
    [[nodiscard]] virtual std::vector<Vertex>   get_vertices() const = 0;
    [[nodiscard]] virtual std::vector<uint32_t> get_indices() const  = 0;
};

class CubePrimitive final : public Primitive
{
  public:
    [[nodiscard]] std::vector<Vertex>   get_vertices() const override;
    [[nodiscard]] std::vector<uint32_t> get_indices() const override;
};

class SquarePrimitive final : public Primitive
{
  public:
    [[nodiscard]] std::vector<Vertex>   get_vertices() const override;
    [[nodiscard]] std::vector<uint32_t> get_indices() const override;
};
} // namespace primitive