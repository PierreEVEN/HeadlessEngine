#pragma once
#include <glm/glm.hpp>

class Box3D
{
  public:
    Box3D(const glm::dvec3& in_min, const glm::dvec3& in_max) : min(in_min), max(in_max)
    {
    }
    Box3D(const glm::dvec3& in_center) : min(in_center), max(in_center)
    {
    }
    Box3D() : min(glm::dvec3(0)), max(glm::dvec3(0))
    {
    }

    Box3D(const Box3D& source, const glm::dmat4& transformation);

    void add_position(const glm::dvec3& in_position);

    [[nodiscard]] const glm::dvec3& get_min() const
    {
        return min;
    }

    [[nodiscard]] const glm::dvec3& get_max() const
    {
        return max;
    }

  private:
    glm::dvec3 min;
    glm::dvec3 max;
};

// see https://gist.github.com/podgorskiy/
class Frustum
{
  public:
    Frustum() = default;

    Frustum(glm::dmat4 view_matrix);

    [[nodiscard]] bool is_box_visible(const Box3D& box) const;

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
