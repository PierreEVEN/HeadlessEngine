#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include "gfx/command_buffer.h"

#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>

#include "gfx/material_instance.h"
#include "gfx/Mesh.h"
#include "gfx/primitives.h"
#include "gfx/view.h"
#include "scene/SubScene.h"

class PlanetRenderer
{
  public:

      struct PlanetVertex
      {
          glm::dvec3 pos;
      };

    struct UboStructure
    {
        glm::dmat4 view_projection_matrix;
    };

    PlanetRenderer()
    {
        planet_material = gfx::MaterialInstance::create(gfx::MasterMaterial::create("data/shaders/planet/planet_material.shb"));

        //std::vector<PlanetVertex> vertices;
        //std::vector<uint32_t>     indices;
        //test_mesh       = std::make_shared<gfx::Mesh>("test mesh", vertices, indices);
        test_mesh = gfx::primitive::cube();
        camera_ubo = gfx::Buffer::create("camera_ubo", 1, sizeof(UboStructure), gfx::EBufferUsage::UNIFORM_BUFFER, gfx::EBufferAccess::CPU_TO_GPU, gfx::EBufferType::IMMEDIATE);
    }

    void tick()
    {
    }

    void pre_render(gfx::View* view)
    {
        camera_ubo->set_data(
            [&view](void* data)
            {
                UboStructure ubo{
                    .view_projection_matrix = view->get_view_matrix(),
                };
                memcpy(data, &ubo, sizeof(ubo));
            });
    }

    void render(gfx::View* view, gfx::CommandBuffer* command_buffer)
    {
        planet_material->bind_buffer("camera_ubo", camera_ubo);        
        command_buffer->draw_mesh(test_mesh.get(), planet_material.get());
    }

  private:
    std::shared_ptr<gfx::Buffer>           camera_ubo;
    std::shared_ptr<gfx::Mesh>             test_mesh;
    std::shared_ptr<gfx::MaterialInstance> planet_material;
};

class Planet : public ecs::Actor
{
  public:
    Planet(double distance_to_origin)
    {
        add_component<PlanetTransform>(distance_to_origin);
        add_component<PlanetRenderer>();
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
        hearth = new_actor<Planet>(10000);
        mars   = hearth->duplicate<Planet>();
        mars->set_orbit_distance(20000);
    }

  private:
    std::shared_ptr<Planet> hearth;
    std::shared_ptr<Planet> mars;
};