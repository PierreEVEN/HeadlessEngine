#pragma once
#include "ecs/actor.h"
#include "ecs/ecs.h"
#include "physics/physic_scene.h"
#include "transform.h"

#include <glm/glm.hpp>

namespace gfx
{
class CommandBuffer;
}

namespace scene
{
// No rigid body physics for global universe. Rigid body are only available for subscenes
class Universe : public ecs::ECS
{
  public:

      Universe();
      virtual ~Universe() override;

    virtual void pre_render()
    {
        ECS::pre_render(global_view);
    }
    virtual void render(gfx::CommandBuffer* command_buffer)
    {
        ECS::render(global_view, command_buffer);
    }

    [[nodiscard]] gfx::View* get_global_view() const
    {
        return global_view;
    }

  private:



    gfx::View* global_view = nullptr;
};

class SubScene : public ecs::ECS
{
  public:
    SubScene()
    {
        physic_scene = physics::Scene::create();
    }

    void tick() override
    {
        ECS::tick();
        physic_scene->step();
    }

  private:
    std::shared_ptr<physics::Scene> physic_scene;
};

} // namespace scene