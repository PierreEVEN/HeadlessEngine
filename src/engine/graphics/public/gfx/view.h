#pragma once

#include "command_buffer.h"
#include "types/frustum.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace gfx
{
class View
{
  public:
    View();

    [[nodiscard]] glm::mat4& get_view_matrix()
    {
        return buffer_data.view_matrix;
    }

    [[nodiscard]] glm::mat4& get_projection_matrix()
    {
        return buffer_data.projection_matrix;
    }

    [[nodiscard]] Frustum& get_view_frustum()
    {
        return view_frustum;
    }

    void set_viewport(float width, float height, float fov, float far, float near)
    {
        buffer_data.projection_matrix = glm::perspective<double>(static_cast<double>(fov), static_cast<double>(width) / static_cast<double>(height), static_cast<double>(near), static_cast<double>(far));
        dirty                         = true;
    }

    void set_view_point(const glm::dvec3& world_pos, const glm::dvec3& forward, const glm::dvec3& up)
    {
        buffer_data.view_matrix = lookAt(world_pos, world_pos + forward, up);
        dirty                   = true;
    }

    const std::shared_ptr<Buffer>& get_buffer();

  private:
    std::shared_ptr<Buffer> camera_buffer;
    Frustum                 view_frustum;

    bool dirty = false;

    struct CameraBufferData
    {
        glm::mat4 view_matrix;
        glm::mat4 projection_matrix;
    } buffer_data;
};
} // namespace gfx