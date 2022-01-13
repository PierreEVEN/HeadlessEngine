#pragma once
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

namespace gfx
{
class Buffer;

class IMeshInterface
{
  public:
    IMeshInterface()                                        = default;
    virtual ~IMeshInterface()                               = default;
    [[nodiscard]] virtual Buffer* get_vertex_buffer() const = 0;
    [[nodiscard]] virtual Buffer* get_index_buffer() const  = 0;
};

class StaticMesh final : IMeshInterface
{
  public:
    struct Vertex
    {
        glm::vec3 pos;
    };

    StaticMesh(const std::string& mesh_name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    ~StaticMesh() override;

    [[nodiscard]] Buffer* get_vertex_buffer() const override
    {
        return vertex_buffer.get();
    }

    [[nodiscard]] Buffer* get_index_buffer() const override
    {
        return index_buffer.get();
    }

  private:
    std::shared_ptr<Buffer> vertex_buffer = nullptr;
    std::shared_ptr<Buffer> index_buffer  = nullptr;
};

class DynamicMesh final : IMeshInterface
{
public:
    [[nodiscard]] Buffer* get_vertex_buffer() const override
    {
        return nullptr;
    }
    [[nodiscard]] Buffer* get_index_buffer() const override
    {
        return nullptr;
    }
};
} // namespace gfx
