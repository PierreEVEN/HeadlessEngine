
#include "types/frustum.h"

namespace gfx
{

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

bool Frustum::intersect(Bound& bounds)
{
    glm::dvec3 minp = bounds.get_box().get_max();
    glm::dvec3 maxp = bounds.get_box().get_max();
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

} // namespace gfx