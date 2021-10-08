
#include "rendering/mesh/vertex.h"


std::vector<VkVertexInputAttributeDescription> VertexInputInfo::get_attributes() const
{
    uint32_t                                       location = 0;
    std::vector<VkVertexInputAttributeDescription> out_attributes;

    for (const auto& attribute : attributes)
    {
        auto description     = attribute.description;
        description.location = location++;
        out_attributes.emplace_back(description);
    }

    return out_attributes;
}

VertexInputInfo Vertex::get_attribute_descriptions()
{
    return VertexInputInfo{

        .vertex_structure_size = sizeof(Vertex),
        .attributes            = {
            VertexInputInfo::VertexAttribute{
                .description =
                    {
                        .location = 0,
                        .binding  = 0,
                        .format   = VK_FORMAT_R32G32B32_SFLOAT,
                        .offset   = offsetof(Vertex, pos),
                    },
                .attribute_name = "vertex_position",
            },
            VertexInputInfo::VertexAttribute{
                .description =
                    {
                        .location = 0,
                        .binding  = 0,
                        .format   = VK_FORMAT_R32G32_SFLOAT,
                        .offset   = offsetof(Vertex, uv),
                    },
                .attribute_name = "vertex_uvs",
            },
            VertexInputInfo::VertexAttribute{
                .description =
                    {
                        .location = 0,
                        .binding  = 0,
                        .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
                        .offset   = offsetof(Vertex, col),
                    },
                .attribute_name = "vertex_color",
            },
            VertexInputInfo::VertexAttribute{
                .description =
                    {
                        .location = 0,
                        .binding  = 0,
                        .format   = VK_FORMAT_R32G32B32_SFLOAT,
                        .offset   = offsetof(Vertex, norm),
                    },
                .attribute_name = "vertex_normal",
            },
            VertexInputInfo::VertexAttribute{
                .description =
                    {
                        .location = 0,
                        .binding  = 0,
                        .format   = VK_FORMAT_R32G32B32_SFLOAT,
                        .offset   = offsetof(Vertex, tang),
                    },
                .attribute_name = "vertex_tangent",
            },
            VertexInputInfo::VertexAttribute{
                .description =
                    {
                        .location = 0,
                        .binding  = 0,
                        .format   = VK_FORMAT_R32G32B32_SFLOAT,
                        .offset   = offsetof(Vertex, bitang),
                    },
                .attribute_name = "vertex_bitang",
            },
        }};
}
