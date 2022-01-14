#pragma once
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
    template <typename Vertex_T>
    StaticMesh(const std::string& mesh_name, std::vector<Vertex_T>& vertices, const std::vector<uint32_t>& indices) : StaticMesh(mesh_name, vertices.data(), vertices.size(), sizeof(Vertex_T), indices)
    {
    }

    ~StaticMesh() override = default;

    [[nodiscard]] Buffer* get_vertex_buffer() const override
    {
        return vertex_buffer.get();
    }

    [[nodiscard]] Buffer* get_index_buffer() const override
    {
        return index_buffer.get();
    }

  private:
    StaticMesh(const std::string& mesh_name, void* vertex_data, uint32_t vertex_count, uint32_t vertex_structure_size, const std::vector<uint32_t>& indices);

    std::shared_ptr<Buffer> vertex_buffer = nullptr;
    std::shared_ptr<Buffer> index_buffer  = nullptr;
};

class DynamicMesh final : IMeshInterface
{
  public:
    template <typename Vertex_T> DynamicMesh(const std::string& mesh_name, const std::vector<Vertex_T>& vertices, const std::vector<uint32_t>& indices) : DynamicMesh(mesh_name, vertices.size(), sizeof(Vertex_T), indices.size())
    {
        set_data(vertices, indices);
    }

    template <typename Vertex_T> DynamicMesh(const std::string& mesh_name) : DynamicMesh(mesh_name, 0, sizeof(Vertex_T), 0)
    {
    }

    template <typename Vertex_T> void set_data(const std::vector<Vertex_T>& vertices, const std::vector<uint32_t>& indices)
    {
        set_data(vertices.data(), vertices.size(), sizeof(Vertex_T), indices);
    }

    [[nodiscard]] Buffer* get_vertex_buffer() const override
    {
        return vertex_buffer.get();
    }
    [[nodiscard]] Buffer* get_index_buffer() const override
    {
        return index_buffer.get();
    }

  private:
    void set_data(const void* vertices, uint32_t vertex_count, uint32_t vertex_size, const std::vector<uint32_t>& indices) const;

    DynamicMesh(const std::string& mesh_name, uint32_t initial_vertex_count, uint32_t vertex_size, uint32_t initial_index_count);

    std::shared_ptr<Buffer> vertex_buffer = nullptr;
    std::shared_ptr<Buffer> index_buffer  = nullptr;
};
} // namespace gfx
