

#include "rendering/debug_draw.h"

#include "assets/asset_mesh_data.h"
#include "assets/asset_shader_buffer.h"
#include "rendering/graphics.h"
#include "rendering/vulkan/common.h"
#include "scene/node_camera.h"

#include <vulkan/vulkan_core.h>

static constexpr const char* vertex_code = "\
#version 460 \n\
layout(location = 0) in vec3 pos; \
layout(location = 1) in vec2 uv; \
layout(location = 2) in vec4 col; \
layout(location = 3) in vec3 norm; \
layout(location = 4) in vec3 tang; \
layout(location = 5) in vec3 bitang; \
layout(binding = 9) uniform GlobalCameraUniformBuffer { \
    mat4                 worldProjection; \
    mat4                 viewMatrix; \
    vec3                 cameraLocation; \
} ubo; \
\
void main() \
{ \
    gl_Position = ubo.worldProjection * ubo.viewMatrix * vec4(pos.xyz, 1.0); \
} \
";

static constexpr const char* fragment_code = " \
#version 460 \n\
#extension GL_ARB_separate_shader_objects : enable \n\
layout(location = 0) out vec4 outColor; \
\
void main() \
{ \
    outColor = vec4(1, 0, 0, 0);\
} \
";

DebugDraw::DebugDraw(NCamera* in_context_camera) : context_camera(in_context_camera)
{
    LOG_WARNING("@TODO : create shader");
}

DebugDraw::~DebugDraw()
{
    for (const auto& buffer : vertex_buffers)
        vkDestroyBuffer(Graphics::get()->get_logical_device(), buffer, vulkan_common::allocation_callback);
    for (const auto& memory : buffer_memories)
        vkFreeMemory(Graphics::get()->get_logical_device(), memory, vulkan_common::allocation_callback);
}

void DebugDraw::draw_line(const glm::dvec3& from, const glm::dvec3& to)
{
    write_lock.lock();
    vertices.emplace_back(Vertex{.pos = from});
    vertices.emplace_back(Vertex{.pos = to});
    write_lock.unlock();
}

void DebugDraw::draw_box(const Box3D& box)
{
    draw_line(glm::dvec3(box.get_min().x, box.get_min().y, box.get_min().z), glm::dvec3(box.get_min().x, box.get_min().y, box.get_max().z));
    draw_line(glm::dvec3(box.get_min().x, box.get_min().y, box.get_min().z), glm::dvec3(box.get_min().x, box.get_max().y, box.get_min().z));
    draw_line(glm::dvec3(box.get_min().x, box.get_min().y, box.get_min().z), glm::dvec3(box.get_max().x, box.get_min().y, box.get_min().z));

    draw_line(glm::dvec3(box.get_max().x, box.get_max().y, box.get_max().z), glm::dvec3(box.get_max().x, box.get_max().y, box.get_min().z));
    draw_line(glm::dvec3(box.get_max().x, box.get_max().y, box.get_max().z), glm::dvec3(box.get_max().x, box.get_min().y, box.get_max().z));
    draw_line(glm::dvec3(box.get_max().x, box.get_max().y, box.get_max().z), glm::dvec3(box.get_min().x, box.get_max().y, box.get_max().z));

    draw_line(glm::dvec3(box.get_max().x, box.get_min().y, box.get_min().z), glm::dvec3(box.get_max().x, box.get_min().y, box.get_max().z));
    draw_line(glm::dvec3(box.get_max().x, box.get_min().y, box.get_min().z), glm::dvec3(box.get_max().x, box.get_max().y, box.get_min().z));
    draw_line(glm::dvec3(box.get_max().x, box.get_max().y, box.get_min().z), glm::dvec3(box.get_min().x, box.get_max().y, box.get_min().z));

    draw_line(glm::dvec3(box.get_min().x, box.get_max().y, box.get_min().z), glm::dvec3(box.get_min().x, box.get_max().y, box.get_max().z));
    draw_line(glm::dvec3(box.get_min().x, box.get_max().y, box.get_max().z), glm::dvec3(box.get_min().x, box.get_min().y, box.get_max().z));
    draw_line(glm::dvec3(box.get_min().x, box.get_min().y, box.get_max().z), glm::dvec3(box.get_max().x, box.get_min().y, box.get_max().z));
}

void DebugDraw::draw_sphere(const glm::dvec3& center, double radius, int subdivisions)
{
    draw_line(center, center + glm::dvec3(radius));
}

void DebugDraw::create_or_resize_buffer(VkBuffer& buffer, VkDeviceMemory& buffer_memory, VkDeviceSize& p_buffer_size, size_t new_size, VkBufferUsageFlags usage)
{
    // free existing buffer
    if (buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(Graphics::get()->get_logical_device(), buffer, vulkan_common::allocation_callback);
    if (buffer_memory != VK_NULL_HANDLE)
        vkFreeMemory(Graphics::get()->get_logical_device(), buffer_memory, vulkan_common::allocation_callback);

    // create buffer
    VkBufferCreateInfo buffer_info{
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = ((new_size - 1) / buffer_memory_alignment + 1) * buffer_memory_alignment,
        .usage       = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VK_ENSURE(vkCreateBuffer(Graphics::get()->get_logical_device(), &buffer_info, vulkan_common::allocation_callback, &buffer));

    // allocate memory
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(Graphics::get()->get_logical_device(), buffer, &requirements);
    buffer_memory_alignment         = (buffer_memory_alignment > requirements.alignment) ? buffer_memory_alignment : requirements.alignment;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize       = requirements.size;
    alloc_info.memoryTypeIndex      = vulkan_utils::find_memory_type(Graphics::get()->get_physical_device(), requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VK_ENSURE(vkAllocateMemory(Graphics::get()->get_logical_device(), &alloc_info, vulkan_common::allocation_callback, &buffer_memory));

    VK_ENSURE(vkBindBufferMemory(Graphics::get()->get_logical_device(), buffer, buffer_memory, 0));
    p_buffer_size = new_size;
}

void DebugDraw::render_wireframe(const SwapchainFrame& in_render_context)
{
    write_lock.lock();
    if (!vertices.empty())
    {
        if (in_render_context.image_index >= vertex_buffers.size())
        {
            for (int64_t i = static_cast<int64_t>(vertex_buffers.size()) - 1; i <= in_render_context.image_index; ++i)
            {
                vertex_buffers.push_back(VK_NULL_HANDLE);
                buffer_memories.push_back(VK_NULL_HANDLE);
                buffer_sizes.emplace_back(0);
            }
        }

        const size_t new_data_size = vertices.size() * sizeof(Vertex);

        create_or_resize_buffer(vertex_buffers[in_render_context.image_index], buffer_memories[in_render_context.image_index], buffer_sizes[in_render_context.image_index], new_data_size,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        // Upload vertex/index data into a single contiguous GPU buffer
        Vertex* vtx_dst = nullptr;
        VK_ENSURE(vkMapMemory(Graphics::get()->get_logical_device(), buffer_memories[in_render_context.image_index], 0, new_data_size, 0, (void**)(&vtx_dst)));
        memcpy(vtx_dst, vertices.data(), new_data_size);
        vkUnmapMemory(Graphics::get()->get_logical_device(), buffer_memories[in_render_context.image_index]);

        std::vector<VkWriteDescriptorSet> write_descriptor_sets = {};
        write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            //.dstSet           = material_pipeline.get_descriptor_sets()[in_render_context.image_index],
            .dstBinding       = 9,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = context_camera->get_scene_uniform_buffer()->get_descriptor_buffer_info(in_render_context.image_index),
            .pTexelBufferView = nullptr,
        });

        vkUpdateDescriptorSets(Graphics::get()->get_logical_device(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
        //vkCmdBindDescriptorSets(in_render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material_pipeline.get_pipeline_layout(), 0, 1, &material_pipeline.get_descriptor_sets()[in_render_context.image_index], 0, nullptr);
        //vkCmdBindPipeline(in_render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material_pipeline.get_pipeline());

        // draw vertices
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(in_render_context.command_buffer, 0, 1, &vertex_buffers[in_render_context.image_index], offsets);
        vkCmdDraw(in_render_context.command_buffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

        vertices.clear();
    }
    write_lock.unlock();
}
