#pragma once

#include "asset_base.h"

#include "misc/Frustum.h"
#include "rendering/mesh/vertex.h"

#include <optional>
#include <vk_mem_alloc.h>

#include <vulkan/vulkan_core.h>

class AMeshData : public AssetBase
{
  public:
    AMeshData(std::vector<Vertex> in_vertices, std::vector<uint32_t> in_indices);
    virtual ~AMeshData();

    [[nodiscard]] const VkBuffer& get_vertex_buffer() const
    {
        return vertex_buffer;
    }

    [[nodiscard]] const VkBuffer& get_index_buffer() const
    {
        return index_buffer;
    }
    [[nodiscard]] uint32_t get_indices_count() const
    {
        return static_cast<uint32_t>(indices.size());
    }

    [[nodiscard]] const Box3D& get_bounds() const
    {
        return local_bounds;
    }

  private:
    void set_mesh_data(const std::vector<Vertex>& in_vertices, const std::vector<uint32_t>& in_indices);

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    VkBuffer          vertex_buffer            = VK_NULL_HANDLE;
    VmaAllocation     vertex_buffer_allocation = VK_NULL_HANDLE;
    VmaAllocationInfo vertex_buffer_alloc_info = {};

    VkBuffer          index_buffer            = VK_NULL_HANDLE;
    VmaAllocation     index_buffer_allocation = VK_NULL_HANDLE;
    VmaAllocationInfo index_buffer_alloc_info = {};

    Box3D local_bounds;
};
