
#include "gfx/StaticMesh.h"

#include "gfx/buffer.h"

#include <cpputils/logger.hpp>

namespace gfx
{

void StaticMesh::set_data(void* vertex_data, uint32_t vertex_count, uint32_t* index_data, uint32_t index_count)
{
    vertex_buffer->resize(index_count);
    index_buffer->set_data(
        [&](void* data)
        {
            memcpy(data, index_data, index_buffer->size());
        });

    vertex_buffer->resize(vertex_count);
    vertex_buffer->set_data(
        [&](void* data)
        {
            memcpy(data, vertex_data, vertex_buffer->size());
        });
}

StaticMesh::StaticMesh(const std::string& mesh_name, uint32_t vertex_structure_size, uint32_t vertex_count, uint32_t index_count, EBufferType buffer_type, EIndexBufferType index_buffer_type)
{
    if (vertex_structure_size == 0)
        LOG_FATAL("cannot create a mesh with a zero sized vertex structure size")
    if (vertex_count == 0)
        vertex_count = 1;
    if (index_count == 0)
        index_count = 1;

    index_buffer  = Buffer::create(mesh_name + "_index_buffer", index_count, static_cast<uint32_t>(index_buffer_type), EBufferUsage::INDEX_DATA, EBufferAccess::CPU_TO_GPU, buffer_type);
    vertex_buffer = Buffer::create(mesh_name + "_vertex_buffer", vertex_count, vertex_structure_size, EBufferUsage::VERTEX_DATA, EBufferAccess::CPU_TO_GPU, buffer_type);
}
} // namespace gfx
