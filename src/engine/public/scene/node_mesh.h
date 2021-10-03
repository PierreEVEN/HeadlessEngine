#pragma once
#include "assets/asset_ptr.h"
#include "misc/Frustum.h"
#include "node_primitive.h"
#include "scene_proxy.h"
#include "types/fast_mutex.h"

class AMeshData;
class AMaterialInstance;

class NMesh : public NPrimitive
{
  public:
    NMesh(TAssetPtr<AMeshData> in_mesh, TAssetPtr<AMaterialInstance> in_material);
    virtual ~NMesh();
    static void register_component(Scene* target_scene);

    void update_data();
    void tick(const double delta_second) override;

    [[nodiscard]] TAssetPtr<AMeshData> get_mesh() const
    {
        return mesh;
    }
    [[nodiscard]] TAssetPtr<AMaterialInstance> get_material() const
    {
        return material;
    }

  protected:
    virtual Box3D get_local_bounds() override;

  private:
    TAssetPtr<AMeshData> mesh;
    TAssetPtr<AMaterialInstance> material;
    FastMutex            proxy_data_lock;
    EntityHandle         proxy_entity_handle;
};