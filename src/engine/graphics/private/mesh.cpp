
#include "gfx/mesh.h"

#include "gfx/buffer.h"

namespace gfx
{

Mesh::Mesh(const std::string& mesh_name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    vertex_buffer = Buffer::create(mesh_name + "_vertex_buffer", static_cast<uint32_t>(vertices.size()), sizeof(Vertex), EBufferUsage::VERTEX_DATA, EBufferAccess::CPU_TO_GPU);
    vertex_buffer->set_data(vertices.data(), vertices.size() * sizeof(Vertex));
    
    index_buffer = Buffer::create(mesh_name + "_index_buffer", static_cast<uint32_t>(indices.size()), sizeof(uint32_t), EBufferUsage::INDEX_DATA, EBufferAccess::CPU_TO_GPU);
    index_buffer->set_data(indices.data(), indices.size() * sizeof(uint32_t));
}

Mesh::~Mesh()
{
}
}
