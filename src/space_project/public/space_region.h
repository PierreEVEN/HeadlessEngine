#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include "application/inputs/input_mapping.h"
#include "planet.h"

#include "scene/SubScene.h"
#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

class Planet : public ecs::Actor
{
  public:
    Planet(double distance_to_origin, std::shared_ptr<gfx::MasterMaterial> planet_material)
    {
        add_component<PlanetTransform>(distance_to_origin);
        add_component<PlanetRenderer>(distance_to_origin, planet_material);
    }

    void set_orbit_distance(double orbit_distance)
    {
        get_component<PlanetTransform>()->set_orbit_distance(orbit_distance);
    }

  private:
    std::vector<std::shared_ptr<Actor>> mesh_sections;

  public:
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
};

class CustomUniverse : public scene::Universe
{
  public:
    CustomUniverse() : Universe()
    {
        planet_material = gfx::MasterMaterial::create("planet_material", "data/shaders/planet/planet_material.shb");

        //planet_material->bind_buffer("camera_ubo", get_global_view()->get_buffer());

        for (int i = 0; i < 1; ++i)
        {
            planets.emplace_back(new_actor<Planet>(i * 5, planet_material));
        }

        move_forward  = std::make_unique<application::inputs::ActionMapping>(application::inputs::EButtons::Keyboard_Z);
        move_backward = std::make_unique<application::inputs::ActionMapping>(application::inputs::EButtons::Keyboard_S);
        move_right    = std::make_unique<application::inputs::ActionMapping>(application::inputs::EButtons::Keyboard_D);
        move_left     = std::make_unique<application::inputs::ActionMapping>(application::inputs::EButtons::Keyboard_Q);
        move_up       = std::make_unique<application::inputs::ActionMapping>(application::inputs::EButtons::Keyboard_Space);
        move_down     = std::make_unique<application::inputs::ActionMapping>(application::inputs::EButtons::Keyboard_LeftShift);

        look_right = std::make_unique<application::inputs::AxisMapping>(application::inputs::EAxis::Mouse_X);
        look_up    = std::make_unique<application::inputs::AxisMapping>(application::inputs::EAxis::Mouse_Y);

        position = glm::dvec3(-10, 0, 0);
    }

    void tick() override
    {
        Universe::tick();

        // glm::dvec3 forward = forward_vector

        glm::dquat rot = glm::eulerAngleYXZ(0.0, static_cast<double>(*look_up->value) * 0.01, static_cast<double>(*look_right->value) * 0.01);

        glm::dvec3 forward = rot * glm::dvec3(1, 0, 0);

        position += glm::dvec3(*move_forward->value ? 1 : *move_backward->value ? -1 : 0, *move_right->value ? -1 : *move_left->value ? 1 : 0, *move_up->value ? 1 : *move_down->value ? -1 : 0) * 0.5;

        get_global_view()->set_viewport(800, 600, 45, 10000, 0.1f);
        get_global_view()->set_view_point(position, forward, glm::dvec3(0, 1, 0));
    }

    void pre_render() override
    {
        Universe::pre_render();
        get_global_view()->get_buffer();
    }

  private:
    std::shared_ptr<gfx::MasterMaterial> planet_material;
    std::vector<std::shared_ptr<Planet>>   planets;

    glm::dvec3                                          position;
    glm::dvec3                                          forward_vector;
    std::unique_ptr<application::inputs::ActionMapping> move_forward;
    std::unique_ptr<application::inputs::ActionMapping> move_backward;
    std::unique_ptr<application::inputs::ActionMapping> move_right;
    std::unique_ptr<application::inputs::ActionMapping> move_left;
    std::unique_ptr<application::inputs::ActionMapping> move_up;
    std::unique_ptr<application::inputs::ActionMapping> move_down;
    std::unique_ptr<application::inputs::AxisMapping>   look_up;
    std::unique_ptr<application::inputs::AxisMapping>   look_right;
};