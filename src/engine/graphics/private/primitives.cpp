#include "gfx/primitives.h"

#include <corecrt_math_defines.h>

namespace gfx::primitive
{
std::shared_ptr<Mesh> cube(float x, float y, float z)
{
    return std::make_shared<Mesh>("generated cube",
                                  std::vector<DefaultVertex>{{
                                      {.pos = glm::dvec3(-x, -y, -z), .uv = glm::vec2(0, 0), .normal = glm::vec3(0, 0, -1)}, {.pos = glm::dvec3(x, -y, -z), .uv = glm::vec2(1, 0), .normal = glm::vec3(0, 0, -1)},
                                      {.pos = glm::dvec3(x, y, -z), .uv = glm::vec2(1, 1), .normal = glm::vec3(0, 0, -1)},   {.pos = glm::dvec3(-x, y, -z), .uv = glm::vec2(0, 1), .normal = glm::vec3(0, 0, -1)},

                                      {.pos = glm::dvec3(-x, -y, z), .uv = glm::vec2(0, 0), .normal = glm::vec3(0, 0, 1)},   {.pos = glm::dvec3(x, -y, z), .uv = glm::vec2(1, 0), .normal = glm::vec3(0, 0, 1)},
                                      {.pos = glm::dvec3(x, y, z), .uv = glm::vec2(1, 1), .normal = glm::vec3(0, 0, 1)},     {.pos = glm::dvec3(-x, y, z), .uv = glm::vec2(0, 1), .normal = glm::vec3(0, 0, 1)},

                                      {.pos = glm::dvec3(x, -y, -z), .uv = glm::vec2(0, 0), .normal = glm::vec3(1, 0, 0)},   {.pos = glm::dvec3(x, -y, z), .uv = glm::vec2(1, 0), .normal = glm::vec3(1, 0, 0)},
                                      {.pos = glm::dvec3(x, y, z), .uv = glm::vec2(1, 1), .normal = glm::vec3(1, 0, 0)},     {.pos = glm::dvec3(x, y, -z), .uv = glm::vec2(0, 1), .normal = glm::vec3(0, 0, 0)},

                                      {.pos = glm::dvec3(-x, -y, -z), .uv = glm::vec2(0, 0), .normal = glm::vec3(1, 0, 0)},  {.pos = glm::dvec3(-x, -y, z), .uv = glm::vec2(1, 0), .normal = glm::vec3(1, 0, 0)},
                                      {.pos = glm::dvec3(-x, y, z), .uv = glm::vec2(1, 1), .normal = glm::vec3(1, 0, 0)},    {.pos = glm::dvec3(-x, y, -z), .uv = glm::vec2(0, 1), .normal = glm::vec3(0, 0, 0)},

                                      {.pos = glm::dvec3(-x, -y, -z), .uv = glm::vec2(0, 0), .normal = glm::vec3(0, 1, 0)},  {.pos = glm::dvec3(-x, -y, z), .uv = glm::vec2(1, 0), .normal = glm::vec3(0, 1, 0)},
                                      {.pos = glm::dvec3(x, -y, z), .uv = glm::vec2(1, 1), .normal = glm::vec3(0, 1, 0)},    {.pos = glm::dvec3(x, -y, -z), .uv = glm::vec2(0, 1), .normal = glm::vec3(0, 1, 0)},

                                      {.pos = glm::dvec3(-x, y, -z), .uv = glm::vec2(0, 0), .normal = glm::vec3(0, -1, 0)},  {.pos = glm::dvec3(-x, y, z), .uv = glm::vec2(1, 0), .normal = glm::vec3(0, -1, 0)},
                                      {.pos = glm::dvec3(x, y, z), .uv = glm::vec2(1, 1), .normal = glm::vec3(0, -1, 0)},    {.pos = glm::dvec3(x, y, -z), .uv = glm::vec2(0, 1), .normal = glm::vec3(0, -1, 0)},
                                  }},
                                  std::vector<uint32_t>{
                                      1, 0, 2, 2, 0, 3, 4, 5, 6, 4, 6, 7, 9, 8, 10, 10, 8, 11, 12, 13, 14, 12, 14, 15, 17, 16, 18, 18, 16, 19, 20, 21, 22, 20, 22, 23,
                                  });
}

std::shared_ptr<Mesh> ico_sphere(float, uint32_t)
{
    return nullptr;
}

std::shared_ptr<Mesh> uv_sphere(float radius, uint32_t subdivision_x, uint32_t subdivision_y)
{
    std::vector<DefaultVertex> vertices;
    std::vector<uint32_t>      indices;

    vertices.reserve(subdivision_x * (subdivision_y * 2 + 1));
    indices.reserve(6 * subdivision_y * 2 * (subdivision_x - 1));

    for (uint32_t i = 0; i < subdivision_x; ++i)
    {
        const float pos_x = cosf(i / static_cast<float>(subdivision_x - 1) * static_cast<float>(M_PI) * 2);
        const float pos_y = sinf(i / static_cast<float>(subdivision_x - 1) * static_cast<float>(M_PI) * 2);

        for (int32_t j = -static_cast<int32_t>(subdivision_y); j <= static_cast<int32_t>(subdivision_y); ++j)
        {
            const float dist   = std::cosf(static_cast<float>(j) / static_cast<float>(subdivision_y) * static_cast<float>(M_PI) / 2);
            const float height = std::sinf(static_cast<float>(j) / static_cast<float>(subdivision_y) * static_cast<float>(M_PI) / 2);

            vertices.emplace_back(DefaultVertex{
                .pos = glm::vec3(pos_x * dist * radius, pos_y * dist * radius, height * radius),
                .uv  = glm::vec2(i / static_cast<float>(subdivision_x), (height + static_cast<int32_t>(subdivision_y)) / 2.f),
            });
        }
    }
    const uint32_t total_subdivision_z = subdivision_y * 2 + 1;
    for (uint32_t i = 0; i < subdivision_x - 1; ++i)
    {
        for (uint32_t j = 0; j < subdivision_y * 2; ++j)
        {
            const uint32_t pos_z = j;

            indices.emplace_back(pos_z + i * total_subdivision_z);
            indices.emplace_back(pos_z + (i + 1) * total_subdivision_z);
            indices.emplace_back(pos_z + i * total_subdivision_z + 1);
            indices.emplace_back(pos_z + i * total_subdivision_z + 1);
            indices.emplace_back(pos_z + (i + 1) * total_subdivision_z);
            indices.emplace_back(pos_z + (i + 1) * total_subdivision_z + 1);
        }
    }

    return std::make_shared<Mesh>("generated uv sphere", vertices, indices);
}
} // namespace gfx::primitive
