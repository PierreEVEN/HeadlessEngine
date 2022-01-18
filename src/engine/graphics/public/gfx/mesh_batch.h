#pragma once

#include "gfx/buffer.h"

#include <glm/glm.hpp>

namespace gfx
{
class MeshBatch
{
  public:
    MeshBatch();
        
    struct MeshData
    {
        glm::dmat4 transform;
    };

  private:
    std::shared_ptr<Buffer> matrix_buffer;
};
} // namespace gfx