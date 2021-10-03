#pragma once
#include "assets/asset_ptr.h"
#include "node_base.h"
#include "rendering/debug_draw.h"

class AShaderBuffer;

class NCamera : public NodeBase
{
  public:
    NCamera();

    [[nodiscard]] glm::dmat4 get_view_matrix() const;

    [[nodiscard]] float get_field_of_view() const
    {
        return field_of_view;
    }

    [[nodiscard]] float get_near_clip_plane() const
    {
        return near_clip_plane;
    }

    [[nodiscard]] float get_far_clip_plane() const
    {
        return far_clip_plane;
    }

    [[nodiscard]] glm::dvec3 get_world_up() const
    {
        return world_up;
    }

    [[nodiscard]] TAssetPtr<AShaderBuffer> get_scene_uniform_buffer() const
    {
        return camera_uniform_buffer;
    }

    [[nodiscard]] TAssetPtr<AShaderBuffer> get_model_ssbo() const
    {
        return global_model_ssbo;
    }

    [[nodiscard]] glm::dmat4 get_world_projection() const
    {
        return world_projection;
    }

    [[nodiscard]] DebugDraw* get_debug_draw() const
    {
        return debug_draws.get();
    }

    void update_view(SwapchainStatus& render_context);

  private:
    TAssetPtr<AShaderBuffer> camera_uniform_buffer = nullptr;
    TAssetPtr<AShaderBuffer> global_model_ssbo     = nullptr;
    float                    near_clip_plane       = 1.f;
    float                    far_clip_plane        = 1000000.f;
    float                    field_of_view         = 45.f;

    glm::dmat4 world_projection = glm::dmat4(1.0);
    glm::dvec3 world_up         = glm::dvec3(0, 0, 1);

    std::unique_ptr<DebugDraw> debug_draws;
};
