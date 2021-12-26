#pragma once

#include "command_buffer.h"
#include "types/frustum.h"

#include <glm/glm.hpp>

namespace gfx
{
class View
{
  public:
    View();

    [[nodiscard]] glm::dmat4& get_view_matrix()
    {
        return view_matrix;
    }

    [[nodiscard]] Frustum& get_view_frustum()
    {
        return view_frustum;
    }

    [[nodiscard]] CommandBuffer& get_command_buffer()
    {
        return command_buffer;
    }

  private:
    Frustum       view_frustum;
    glm::dmat4    view_matrix;
    CommandBuffer command_buffer;
};
} // namespace gfx