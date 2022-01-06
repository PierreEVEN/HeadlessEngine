
#include "gfx/mesh.h"

#include "gfx/buffer.h"

namespace gfx
{

Mesh::Mesh(const std::string& mesh_name)
{
    vertex_buffer = new Buffer(mesh_name + "_vertex_buffer", , EBufferUsage::VERTEX_DATA, EBufferAccess::CPU_TO_GPU);

}

Mesh::~Mesh()
{
    delete vertex_buffer;
    delete index_buffer;
}
}
