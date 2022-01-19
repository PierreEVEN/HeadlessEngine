#pragma once
#include "transform.h"
#include "ecs/actor.h"
#include "ecs/ecs.h"

#include <glm/glm.hpp>

namespace scene
{
class Scene
{
  public:
    Scene() = default;
    [[nodiscard]] const ecs::ECS& get_ecs() const
    {
        return scene_ecs;
    }

    void make_view(const glm::dmat4& world_view_point)
    {
        
    }

    ecs::Actor add_actor()
    {
        return ecs::Actor();
    }

  private:
    glm::dmat4 scene_transform;
    ecs::ECS   scene_ecs;
};

void universe_declaration_test()
{
    Scene earth_universe;
    Scene mars_universe;


    ecs::Actor test_ship_base = earth_universe.add_actor();

    test_ship_base.add_component<Transform>();


}



} // namespace scene