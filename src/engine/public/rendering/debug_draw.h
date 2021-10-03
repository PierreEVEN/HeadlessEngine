#pragma once

#include "assets/asset_mesh_data.h"
#include "misc/Frustum.h"
#include "rendering/renderer/swapchain.h"
#include "types/fast_mutex.h"
#include "vulkan/material_pipeline.h"

#include <glm/glm.hpp>
#include <vector>

class NCamera;

class DebugDraw final
{
  public:
    DebugDraw(NCamera* in_context_camera);
    ~DebugDraw();

    void draw_line(const glm::dvec3& from, const glm::dvec3& to);
    void draw_box(const Box3D& box);
    void draw_sphere(const glm::dvec3& center, double radius, int subdivisions = 5);

    void render_wireframe(const SwapchainFrame& in_render_context);

  private:
    void create_or_resize_buffer(VkBuffer& buffer, VkDeviceMemory& buffer_memory, VkDeviceSize& p_buffer_size, size_t new_size, VkBufferUsageFlags usage);

    std::vector<Vertex>         vertices;
    std::vector<VkBuffer>       vertex_buffers;
    std::vector<VkDeviceMemory> buffer_memories;
    std::vector<VkDeviceSize>   buffer_sizes;

    size_t buffer_memory_alignment = 256;

    FastMutex write_lock;

    MaterialPipeline        material_pipeline;

    NCamera* context_camera = nullptr;
};
