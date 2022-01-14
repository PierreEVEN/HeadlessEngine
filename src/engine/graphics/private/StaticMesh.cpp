
#include "gfx/StaticMesh.h"

#include "gfx/buffer.h"

namespace gfx
{

StaticMesh::StaticMesh(const std::string& mesh_name, void* vertex_data, uint32_t vertex_count, uint32_t vertex_structure_size, const std::vector<uint32_t>& indices)
{
    vertex_buffer = Buffer::create(mesh_name + "_vertex_buffer", vertex_count, vertex_structure_size, EBufferUsage::VERTEX_DATA, EBufferAccess::CPU_TO_GPU);
    index_buffer  = Buffer::create(mesh_name + "_index_buffer", static_cast<uint32_t>(indices.size()), sizeof(uint32_t), EBufferUsage::INDEX_DATA, EBufferAccess::CPU_TO_GPU);
    vertex_buffer->set_data(vertex_data, vertex_count * vertex_structure_size);
    index_buffer->set_data(indices.data(), indices.size() * sizeof(uint32_t));
}

void DynamicMesh::set_data(const void* vertices, uint32_t vertex_count, uint32_t vertex_size, const std::vector<uint32_t>& indices) const
{
    vertex_buffer->set_data(vertices, vertex_count * vertex_size, 0);
    index_buffer->set_data(indices.data(), indices.size() * sizeof(uint32_t), 0);
}

DynamicMesh::DynamicMesh(const std::string& mesh_name, uint32_t initial_vertex_count, uint32_t vertex_size, uint32_t initial_index_count)
{
    vertex_buffer = gfx::Buffer::create(mesh_name + "_vertex_buffer", initial_vertex_count, vertex_size, EBufferUsage::VERTEX_DATA, EBufferAccess::CPU_TO_GPU);
    index_buffer  = gfx::Buffer::create(mesh_name + "_index_buffer", initial_index_count, sizeof(uint32_t), EBufferUsage::INDEX_DATA, EBufferAccess::CPU_TO_GPU);
}
} // namespace gfx
