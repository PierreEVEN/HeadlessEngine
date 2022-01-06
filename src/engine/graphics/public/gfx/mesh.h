#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/vec3.hpp>

namespace gfx
{



class Buffer;
class Mesh
{
  public:
    //@TODO : improve vertex structure system
    struct Vertex
    {
        glm::vec3 pos;
    };

    Mesh(const std::string& mesh_name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    virtual ~Mesh();

    [[nodiscard]] Buffer* get_vertex_buffer() const
    {
        return vertex_buffer.get();
    }

    [[nodiscard]] Buffer* get_index_buffer() const
    {
        return index_buffer.get();
    }
  private:
    std::shared_ptr<Buffer> vertex_buffer = nullptr;
    std::shared_ptr<Buffer> index_buffer  = nullptr;
};
} // namespace gfx
