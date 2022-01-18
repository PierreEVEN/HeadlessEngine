#include "gfx/mesh_batch.h"

namespace gfx
{
MeshBatch::MeshBatch()
{
    matrix_buffer = Buffer::create("matrix buffer", 0, sizeof(MeshData), EBufferUsage::GPU_MEMORY, EBufferAccess::CPU_TO_GPU, EBufferType::IMMEDIATE);
}
} // namespace gfx