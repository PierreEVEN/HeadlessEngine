#pragma once
#include "buffer.h"

#include <memory>
#include <string>
#include <vector>

namespace gfx
{
class Buffer;

class StaticMesh
{
  public:
    StaticMesh(const std::string& mesh_name, uint32_t vertex_count, uint32_t vertex_structure_size, uint32_t index_count = 0, EBufferType buffer_type = EBufferType::STATIC);

    virtual ~StaticMesh() = default;

    [[nodiscard]] Buffer* get_vertex_buffer() const
    {
        return vertex_buffer.get();
    }

    [[nodiscard]] Buffer* get_index_buffer() const
    {
        return index_buffer.get();
    }

    void set_data(void* vertex_data, uint32_t vertex_count, uint32_t* index_data, uint32_t index_count);
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
