#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>

#include "gfx/material_instance.h"
#include "scene/SubScene.h"

namespace gfx
{
class CommandBuffer;

class ViewStr
{
  public:
    glm::mat4 proj_matrix;
    glm::mat4 view_matrix;
    float     end;
};
} // namespace gfx

class PlanetTransform : public scene::Transform
{
  public:
    PlanetTransform(double distance_to_origin) : distance(distance_to_origin)
    {
    }

    void tick()
    {
        rotation += 1 / 60.0;
        set_world_position(glm::dvec3(cos(rotation) * distance, sin(rotation) * distance, 0));
    }

    void set_orbit_distance(double orbit_distance)
    {
        distance = orbit_distance;
    }

  private:
    double rotation;
    double distance;
};

class Planet : public ecs::Actor
{
  public:
    Planet(double distance_to_origin)
    {
        add_component<PlanetTransform>(distance_to_origin);
    }

    void set_orbit_distance(double orbit_distance)
    {
        get_component<PlanetTransform>()->set_orbit_distance(orbit_distance);
    }
};

class CustomUniverse : public scene::Universe
{
  public:
    CustomUniverse() : Universe()
    {
        hearth = new_actor<Planet>(10000);
        mars   = hearth->duplicate<Planet>();
        mars->set_orbit_distance(20000);
    }

  private:
    std::shared_ptr<Planet> hearth;
    std::shared_ptr<Planet> mars;
};