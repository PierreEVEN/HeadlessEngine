#pragma once

#include "asset_base.h"

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "misc/Frustum.h"

#include <glm/glm.hpp>
#include <vk_mem_alloc.h>

#include <vulkan/vulkan_core.h>

struct Vertex
{
    glm::vec3 pos    = glm::vec3(0);
    glm::vec2 uv     = glm::vec2(0);
    glm::vec4 col    = glm::vec4(1);
    glm::vec3 norm   = glm::vec3(0);
    glm::vec3 tang   = glm::vec3(0);
    glm::vec3 bitang = glm::vec3(0);

    static VkVertexInputBindingDescription get_binding_description();

    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
};

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
