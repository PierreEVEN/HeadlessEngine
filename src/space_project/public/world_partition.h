#pragma once

#include <glm/glm.hpp>

class ViewPoint
{
    
};


class WorldPartition final
{
  public:
    WorldPartition();
    virtual ~WorldPartition() = default;

    void include(WorldPartition& other);
    void render(const ViewPoint& view_point);


  private:
    glm::dvec3 velocity;
    glm::dvec3 aabb_min;
    glm::dvec3 aabb_max;
};
