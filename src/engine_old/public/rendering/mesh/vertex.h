#pragma once

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

struct VertexInputInfo
{
    struct VertexAttribute
    {
        VkVertexInputAttributeDescription description    = {};
        std::string                       attribute_name = "";
    };

    uint32_t                     vertex_structure_size = 0;
    std::vector<VertexAttribute> attributes            = {};

    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> get_attributes() const;
};

struct Vertex
{
    glm::vec3 pos    = glm::vec3(0);
    glm::vec2 uv     = glm::vec2(0);
    glm::vec4 col    = glm::vec4(1);
    glm::vec3 norm   = glm::vec3(0);
    glm::vec3 tang   = glm::vec3(0);
    glm::vec3 bitang = glm::vec3(0);

    static VertexInputInfo get_attribute_descriptions();
};
