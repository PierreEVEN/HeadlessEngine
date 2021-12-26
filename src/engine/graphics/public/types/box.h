#pragma once

#include "sphere.h"

namespace gfx
{
class Box
{
  public:
    Box() : box_min(glm::dvec3(0)), box_max(glm::dvec3(0)), is_initialized(false)
    {
    }
    Box(const glm::dvec3& min, const glm::dvec3& max) : box_min(min), box_max(max), is_initialized(true)
    {
    }
    Box(const Sphere& other) : is_initialized(true)
    {
        const auto& center     = other.get_center();
        auto        radius     = other.get_radius();
        double      box_radius = sqrt(radius * radius + radius * radius);

        box_max = center + glm::dvec3(box_radius);
        box_min = center - glm::dvec3(box_radius);
    }

    [[nodiscard]] glm::dvec3 get_size() const
    {
        return box_max - box_min;
    }

    [[nodiscard]] Sphere sphere() const
    {
        const auto  center = (box_max + box_min) / glm::dvec3(2.0);
        const auto& size   = get_size();
        double      radius = sqrt(size.x * size.x + size.y * size.y + size.z * size.z) / 2;

        return Sphere(center, radius);
    }

    [[nodiscard]] const glm::dvec3& get_min() const
    {
        return box_min;
    }

    [[nodiscard]] const glm::dvec3& get_max() const
    {
        return box_max;
    }

    void set_center(const glm::dvec3& center)
    {
        const auto& half_size = get_size() / glm::dvec3(2);
        box_min               = center - half_size;
        box_max               = center + half_size;
    }

  private:
    bool       is_initialized;
    glm::dvec3 box_min;
    glm::dvec3 box_max;
};
} // namespace gfx