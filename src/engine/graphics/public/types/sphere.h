#pragma once

#include <glm/glm.hpp>

namespace gfx
{
class Sphere
{
  public:
    Sphere(const glm::dvec3& sphere_center, double sphere_radius) : center(sphere_center), radius(sphere_radius)
    {
    }

    Sphere()
    {
    }

    [[nodiscard]] double get_radius() const
    {
        return radius;
    }

    void set_radius(double sphere_radius)
    {
        radius = sphere_radius;
    }

    [[nodiscard]] const glm::dvec3& get_center() const
    {
        return center;
    }

    void set_center(glm::dvec3 sphere_center)
    {
        center = sphere_center;
    }

  private:
    double     radius;
    glm::dvec3 center;
};
} // namespace gfx