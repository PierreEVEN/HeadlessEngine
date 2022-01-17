
#include "misc/Frustum.h"

Box3D::Box3D(const Box3D& source, const glm::dmat4& transformation)
{
    min = transformation * glm::dvec4(source.min, 1.0);
    max = transformation * glm::dvec4(source.max, 1.0);
    if (min.x > max.x)
    {
        const double temp = max.x;
        max.x             = min.x;
        min.x             = temp;
    }
    if (min.y > max.y)
    {
        const double temp = max.y;
        max.y             = min.y;
        min.y             = temp;
    }
    if (min.z > max.z)
    {
        const double temp = max.z;
        max.z             = min.z;
        min.z             = temp;
    }
}

void Box3D::add_position(const glm::dvec3& in_position)
{
    if (in_position.x < min.x)
        min.x = in_position.x;
    if (in_position.y < min.y)
        min.y = in_position.y;
    if (in_position.z < min.z)
        min.z = in_position.z;
    if (in_position.x > max.x)
        max.x = in_position.x;
    if (in_position.y > max.y)
        max.y = in_position.y;
    if (in_position.z > max.z)
        max.z = in_position.z;
}

Frustum::Frustum(glm::dmat4 view_matrix)
{
    view_matrix    = glm::transpose(view_matrix);
    planes[Left]   = view_matrix[3] + view_matrix[0];
    planes[Right]  = view_matrix[3] - view_matrix[0];
    planes[Bottom] = view_matrix[3] + view_matrix[1];
    planes[Top]    = view_matrix[3] - view_matrix[1];
    planes[Near]   = view_matrix[3] + view_matrix[2];
    planes[Far]    = view_matrix[3] - view_matrix[2];

    glm::dvec3 crosses[Combinations] = {
        cross(glm::dvec3(planes[Left]), glm::dvec3(planes[Right])), cross(glm::dvec3(planes[Left]), glm::dvec3(planes[Bottom])), cross(glm::dvec3(planes[Left]), glm::dvec3(planes[Top])),
        cross(glm::dvec3(planes[Left]), glm::dvec3(planes[Near])),  cross(glm::dvec3(planes[Left]), glm::dvec3(planes[Far])),    cross(glm::dvec3(planes[Right]), glm::dvec3(planes[Bottom])),
        cross(glm::dvec3(planes[Right]), glm::dvec3(planes[Top])),  cross(glm::dvec3(planes[Right]), glm::dvec3(planes[Near])),  cross(glm::dvec3(planes[Right]), glm::dvec3(planes[Far])),
        cross(glm::dvec3(planes[Bottom]), glm::dvec3(planes[Top])), cross(glm::dvec3(planes[Bottom]), glm::dvec3(planes[Near])), cross(glm::dvec3(planes[Bottom]), glm::dvec3(planes[Far])),
        cross(glm::dvec3(planes[Top]), glm::dvec3(planes[Near])),   cross(glm::dvec3(planes[Top]), glm::dvec3(planes[Far])),     cross(glm::dvec3(planes[Near]), glm::dvec3(planes[Far]))};

    points[0] = intersection<Left, Bottom, Near>(crosses);
    points[1] = intersection<Left, Top, Near>(crosses);
    points[2] = intersection<Right, Bottom, Near>(crosses);
    points[3] = intersection<Right, Top, Near>(crosses);
    points[4] = intersection<Left, Bottom, Far>(crosses);
    points[5] = intersection<Left, Top, Far>(crosses);
    points[6] = intersection<Right, Bottom, Far>(crosses);
    points[7] = intersection<Right, Top, Far>(crosses);
}

bool Frustum::is_box_visible(const Box3D& box) const
{
    glm::dvec3 minp = box.get_min();
    glm::dvec3 maxp = box.get_max();
    // check box outside/inside of frustum
    for (int i = 0; i < Count; i++)
    {
        if (dot(planes[i], glm::dvec4(minp.x, minp.y, minp.z, 1.0f)) < 0.0 && dot(planes[i], glm::dvec4(maxp.x, minp.y, minp.z, 1.0f)) < 0.0 && dot(planes[i], glm::dvec4(minp.x, maxp.y, minp.z, 1.0f)) < 0.0 &&
            dot(planes[i], glm::dvec4(maxp.x, maxp.y, minp.z, 1.0f)) < 0.0 && dot(planes[i], glm::dvec4(minp.x, minp.y, maxp.z, 1.0f)) < 0.0 && dot(planes[i], glm::dvec4(maxp.x, minp.y, maxp.z, 1.0f)) < 0.0 &&
            dot(planes[i], glm::dvec4(minp.x, maxp.y, maxp.z, 1.0f)) < 0.0 && dot(planes[i], glm::dvec4(maxp.x, maxp.y, maxp.z, 1.0f)) < 0.0)
        {
            return false;
        }
    }

    // check frustum outside/inside box
    int out;
    out = 0;
    for (auto m_point : points)
        out += ((m_point.x > maxp.x) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (auto m_point : points)
        out += ((m_point.x < minp.x) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (auto m_point : points)
        out += ((m_point.y > maxp.y) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (auto m_point : points)
        out += ((m_point.y < minp.y) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (auto m_point : points)
        out += ((m_point.z > maxp.z) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (auto m_point : points)
        out += ((m_point.z < minp.z) ? 1 : 0);
    if (out == 8)
        return false;

    return true;
}
