#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include "application/application.h"
#include "application/inputs/input_mapping.h"
#include "planet.h"
#include "gfx/resource/device.h"

#include "scene/SubScene.h"
#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#define M_PI 3.14159265358979323846

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
        planet_material  = gfx::MasterMaterial::create("planet_material", "data/shaders/planet/planet_material.shb");
        default_material = gfx::MaterialInstance::create(gfx::MasterMaterial::create("planet_material", "data/shaders/default_material.shb"));
        cube_mesh        = gfx::primitive::cube(50, 10, 0.1f);

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

        position = glm::dvec3(0, 0, 10);
    }

    void tick() override
    {
        Universe::tick();

        constexpr double dpi = 0.002;
        yaw += (mouse_last_x - static_cast<double>(*look_right->value)) * dpi;
        pitch = std::clamp(pitch + (static_cast<double>(*look_up->value) - mouse_last_y) * dpi, -M_PI / 2, M_PI / 2);

        mouse_last_x = static_cast<double>(*look_right->value);
        mouse_last_y = static_cast<double>(*look_up->value);

        const glm::dquat rot = glm::eulerAngleZXY(yaw, 0.0, pitch);

        const glm::dvec3 forward = rot * glm::dvec3(1, 0, 0);
        const glm::dvec3 right   = rot * glm::dvec3(0, 1, 0);
        const glm::dvec3 up      = rot * glm::dvec3(0, 0, 1);
        position += ((*move_forward->value    ? forward
                      : *move_backward->value ? -forward
                                              : glm::dvec3()) +
                     (*move_right->value  ? -right
                      : *move_left->value ? right
                                          : glm::dvec3()) +
                     (*move_up->value     ? up
                      : *move_down->value ? -up
                                          : glm::dvec3())) *
                    glm::dvec3(5 * application::get()->delta_time());

        get_global_view()->set_viewport(application::window::Window::get_window(0)->absolute_width(), application::window::Window::get_window(0)->absolute_height(), 45, 10000, 0.1f);
        get_global_view()->set_view_point(position, forward, glm::dvec3(0, 0, 1));
    }

    void pre_render() override
    {
        Universe::pre_render();
        get_global_view()->get_buffer();

        default_material->bind_buffer("camera_ubo", get_global_view()->get_buffer());
    }

    void render(gfx::CommandBuffer* cmd) override
    {
        // Universe::render(cmd);
        cmd->draw_mesh(cube_mesh.get(), default_material.get());
        cmd->draw_mesh(sphere_mesh.get(), default_material.get());
    }

  private:
    std::shared_ptr<gfx::MaterialInstance> default_material;

    std::shared_ptr<gfx::Mesh>           cube_mesh;
    std::shared_ptr<gfx::Mesh>           sphere_mesh = gfx::primitive::uv_sphere();
    std::shared_ptr<gfx::MasterMaterial> planet_material;
    std::vector<std::shared_ptr<Planet>> planets;

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

    double pitch        = 0;
    double yaw          = 0;
    double mouse_last_y = 0;
    double mouse_last_x = 0;
};