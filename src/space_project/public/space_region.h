#pragma once

#include "gfx/buffer.h"
#include "gfx/command_buffer.h"
#include "gfx/materials/master_material.h"
#include "gfx/materials/material_instance.h"
#include "gfx/mesh.h"
#include "gfx/render_pass_reference.h"

#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>

namespace gfx
{
class CommandBuffer;
struct View
{
    glm::dquat global_rotation;
    glm::dvec3 global_location;
    double     fov;
};
} // namespace gfx

struct ECSInstance
{
    void pre_render()
    {
    }
    void render([[maybe_unused]] gfx::CommandBuffer* command_buffer)
    {
    }

    void tick()
    {
    }
};

class SpaceRegion
{
  public:
    SpaceRegion()
    {
        auto vertices = std::vector{gfx::Mesh::Vertex{
                                        .pos = glm::vec3(0, 0, 0),
                                    },
                                    gfx::Mesh::Vertex{
                                        .pos = glm::vec3(1, 0, 0),
                                    },
                                    gfx::Mesh::Vertex{
                                        .pos = glm::vec3(1, 1, 0),
                                    },
                                    gfx::Mesh::Vertex{
                                        .pos = glm::vec3(0, 1, 0),
                                    }};
        auto indices  = std::vector<uint32_t>{0, 2, 1, 0, 3, 2};
        test_mesh     = std::make_shared<gfx::Mesh>("test", vertices, indices);

        test_material_base = gfx::MasterMaterial::create("data/shaders/draw_procedural_test.shb");
        test_material      = gfx::MaterialInstance::create(test_material_base);

        view_matrix_uniform_buffer = gfx::Buffer::create("test_ubo", 1, sizeof(gfx::View), gfx::EBufferUsage::UNIFORM_BUFFER, gfx::EBufferAccess::CPU_TO_GPU);

        gfx::View view;

        view_matrix_uniform_buffer->set_data(view);
        test_material->bind_buffer("view_ubo", view_matrix_uniform_buffer);
    }

    void pre_render([[maybe_unused]] gfx::View* view_point)
    {
        ecs.pre_render();
    }

    void render(gfx::CommandBuffer* command_buffer)
    {
        command_buffer->draw_mesh(test_mesh.get(), test_material.get());
        ecs.render(command_buffer);
    }

    void tick()
    {
        ecs.tick();
    }

  private:
    // Contains all the actors that are currently contained in the region
    ECSInstance ecs;

    std::shared_ptr<gfx::Buffer> view_matrix_uniform_buffer;

    std::shared_ptr<gfx::Mesh>             test_mesh;
    std::shared_ptr<gfx::MaterialInstance> test_material;
    std::shared_ptr<gfx::MasterMaterial>   test_material_base;

    double     region_radius; // for bound checking
    glm::dquat region_global_rotation;
    glm::dvec3 region_global_position;
};
