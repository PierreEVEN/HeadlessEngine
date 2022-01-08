#pragma once

#include "gfx/buffer.h"
#include "gfx/command_buffer.h"
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
    void render(gfx::CommandBuffer* command_buffer)
    {
    }

    void tick()
    {
    }
};

class SpaceRegion
{
  public:
    void pre_render(gfx::View* view_point)
    {
        // set view matrix (using the world-to-local conversion)
        //
        // view_matrix_buffer.set_data(region_to_world_view(view_point));
        ecs.pre_render();
    }

    //@before : command_buffer->start(); // clear last pass data
    void render(gfx::CommandBuffer* command_buffer)
    {
        // command_buffer.set_uniform("view_matrix", view_matrix_buffer);
        ecs.render(command_buffer);
    }
    //@after : command_buffer->submit(); // submit data

    void tick()
    {
        ecs.tick();
    }

  private:

    // Contains all the actors that are currently contained in the region
    ECSInstance ecs;

    gfx::RenderPassData<gfx::Buffer> view_matrix_uniform_buffer;

    double     region_radius; // for bound checking
    glm::dquat region_global_rotation;
    glm::dvec3 region_global_position;
};
