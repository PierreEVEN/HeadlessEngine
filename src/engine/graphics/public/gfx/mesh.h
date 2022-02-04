#pragma once
#include "buffer.h"

#include <memory>
#include <string>
#include <vector>
#include <cpputils/logger.hpp>

#include <glm/glm.hpp>

namespace gfx
{
class Buffer;

struct DefaultVertex
{
    glm::dvec3 pos;
    glm::vec2  uv;
    glm::vec4  color;
    glm::vec3  normal;
    glm::vec3  tangent;
    glm::vec3  bi_tangent;
};

enum class EIndexBufferType
{
    UINT16 = sizeof(uint16_t),
    UINT32 = sizeof(uint32_t),
};

class Mesh
{
  public:
    Mesh(const std::string& mesh_name, uint32_t vertex_structure_size, uint32_t vertex_count = 0, uint32_t index_count = 0, EBufferType buffer_type = EBufferType::IMMUTABLE,
         EIndexBufferType index_buffer_type = EIndexBufferType::UINT32);

    Mesh(const std::string& mesh_name, const void* vertex_data, uint32_t vertex_structure_size, uint32_t vertex_count, const void* index_data, uint32_t index_count, EBufferType buffer_type = EBufferType::IMMUTABLE,
         EIndexBufferType index_buffer_type = EIndexBufferType::UINT32);

    template <typename Vertex_T>
    Mesh(const std::string& mesh_name, std::vector<Vertex_T> vertices, std::vector<uint32_t> indices, EBufferType buffer_type = EBufferType::IMMUTABLE, EIndexBufferType index_buffer_type = EIndexBufferType::UINT32)
        : Mesh(mesh_name, vertices.data(), sizeof(Vertex_T), static_cast<uint32_t>(vertices.size()), indices.data(), static_cast<uint32_t>(indices.size()), buffer_type, index_buffer_type)
    {
    }

    virtual ~Mesh() = default;

    [[nodiscard]] Buffer* get_vertex_buffer() const
    {
        return vertex_buffer.get();
    }

    [[nodiscard]] Buffer* get_index_buffer() const
    {
        return index_buffer.get();
    }

    void                              set_data(void* vertex_data, uint32_t vertex_count, uint32_t* index_data, uint32_t index_count);
    template <typename Vertex_T> void set_data(const std::vector<Vertex_T>& vertex_data, const std::vector<uint32_t>& index_data)
    {
        set_data(vertex_data.data(), vertex_data.size(), index_data.data(), index_data.size());
    }
    template <typename Lambda_T> void set_data(uint32_t vertex_count, uint32_t index_count, Lambda_T callback)
    {
        vertex_buffer->resize(vertex_count);
        index_buffer->resize(index_count);
        vertex_buffer->set_data(
            [&](void* vertex_data)
            {
                index_buffer->set_data(
                    [&](void* index_data)
                    {
                        callback(vertex_data, index_data);
                    });
            });
    }

  private:
    std::shared_ptr<Buffer> vertex_buffer = nullptr;
    std::shared_ptr<Buffer> index_buffer  = nullptr;
};
} // namespace gfx
