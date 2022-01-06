#pragma once
#include <string>

namespace gfx
{
class Buffer;
class Mesh
{
  public:
    Mesh(const std::string& mesh_name);
    virtual ~Mesh();

    [[nodiscard]] Buffer* get_vertex_buffer() const
    {
        return vertex_buffer;
    }

    [[nodiscard]] Buffer* get_index_buffer() const
    {
        return vertex_buffer;
    }
  private:
    Buffer* vertex_buffer = nullptr;
    Buffer* index_buffer  = nullptr;
};
} // namespace gfx
