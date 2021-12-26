#pragma once

#include <glm/glm.hpp>

#include "types/bound.h"

namespace gfx
{
class Frustum
{
  public:
    Frustum() = default;

    Frustum(glm::dmat4 view_matrix);

    [[nodiscard]] bool intersect(Bound& bounds);

  private:
    enum Planes
    {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        Count,
        Combinations = Count * (Count - 1) / 2
    };

    template <Planes i, Planes j> struct ij2k
    {
        enum
        {
            k = i * (9 - i) / 2 + j - 1
        };
    };

    glm::dvec4 planes[Count];
    glm::dvec3 points[8];

    template <Planes a, Planes b, Planes c> [[nodiscard]] glm::dvec3 intersection(const glm::dvec3* crosses) const
    {
        double     d   = glm::dot(glm::dvec3(planes[a]), crosses[ij2k<b, c>::k]);
        glm::dvec3 res = glm::mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) * glm::dvec3(planes[a].w, planes[b].w, planes[c].w);
        return res * (-1.0f / d);
    }
};
} // namespace gfx