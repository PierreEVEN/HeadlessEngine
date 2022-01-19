#include "gfx/primitives.h"
namespace gfx::primitive
{
std::shared_ptr<Mesh> cube(float width)
{
    return std::make_shared<Mesh>("generated cube",
                                  std::vector<DefaultVertex>{{
                                      {
                                          .pos    = glm::dvec3(-width, -width, -width),
                                          .uv     = glm::vec2(0, 0),
                                          .normal = glm::vec3(1, 0, 0),
                                      },
                                      {
                                          .pos    = glm::dvec3(width, -width, -width),
                                          .uv     = glm::vec2(1, 0),
                                          .normal = glm::vec3(1, 0, 0),
                                      },
                                      {
                                          .pos    = glm::dvec3(width, width, -width),
                                          .uv     = glm::vec2(1, 1),
                                          .normal = glm::vec3(1, 0, 0),
                                      },
                                      {
                                          .pos    = glm::dvec3(-width, width, -width),
                                          .uv     = glm::vec2(0, 1),
                                          .normal = glm::vec3(1, 0, 0),
                                      },

                                      {
                                          .pos    = glm::dvec3(-width, -width, width),
                                          .uv     = glm::vec2(0, 0),
                                          .normal = glm::vec3(1, 0, 0),
                                      },
                                      {
                                          .pos    = glm::dvec3(width, -width, width),
                                          .uv     = glm::vec2(1, 0),
                                          .normal = glm::vec3(1, 0, 0),
                                      },
                                      {
                                          .pos    = glm::dvec3(width, width, width),
                                          .uv     = glm::vec2(1, 1),
                                          .normal = glm::vec3(1, 0, 0),
                                      },
                                      {
                                          .pos    = glm::dvec3(-width, width, width),
                                          .uv     = glm::vec2(0, 1),
                                          .normal = glm::vec3(1, 0, 0),
                                      },
                                  }},
                                  std::vector<uint32_t>{
                                      0,
                                      1,
                                      2,
                                      0,
                                      2,
                                      3,
                                      4,
                                      5,
                                      6,
                                      4,
                                      6,
                                      7,
                                  });
}

std::shared_ptr<Mesh> ico_sphere(float, uint32_t)
{
    return nullptr;
}

std::shared_ptr<Mesh> uv_sphere(float, uint32_t, uint32_t)
{
    return nullptr;
}
} // namespace gfx::primitive
