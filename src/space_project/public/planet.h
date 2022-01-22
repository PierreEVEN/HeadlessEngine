#pragma once
#include "gfx/primitives.h"

#include <gfx/gfx.h>

class PlanetRenderer
{
  public:
    struct PlanetVertex
    {
        glm::vec3 pos;
    };

    std::shared_ptr<gfx::Mesh> generate_mesh(float size)
    {
        float                     width    = size;
        std::vector<PlanetVertex> vertices = std::vector<PlanetVertex>{{
            {
                .pos    = glm::dvec3(-width, -width, -width),
            },
            {
                .pos    = glm::dvec3(width, -width, -width),
            },
            {
                .pos    = glm::dvec3(width, width, -width),
            },
            {
                .pos    = glm::dvec3(-width, width, -width),
            },

            {
                .pos    = glm::dvec3(-width, -width, width),
            },
            {
                .pos    = glm::dvec3(width, -width, width),
            },
            {
                .pos    = glm::dvec3(width, width, width),
            },
            {
                .pos    = glm::dvec3(-width, width, width),
            },
        }};
        std::vector<uint32_t> indices = std::vector<uint32_t>{
            0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7,
        };
        return std::make_shared<gfx::Mesh>("test mesh", vertices, indices, gfx::EBufferType::IMMUTABLE);
    }


    PlanetRenderer(float width, std::shared_ptr<gfx::MaterialInstance> planet_mat) : planet_material(planet_mat)
    {
        test_mesh = generate_mesh(width);
    }

    ~PlanetRenderer()
    {
    }

    void tick()
    {
    }

    void pre_render(gfx::View* view)
    {
    }

    void render(gfx::View* view, gfx::CommandBuffer* command_buffer)
    {
        command_buffer->draw_mesh(test_mesh.get(), planet_material.get());
    }

  private:
    std::shared_ptr<gfx::Mesh>             test_mesh;
    std::shared_ptr<gfx::MaterialInstance> planet_material;
};
