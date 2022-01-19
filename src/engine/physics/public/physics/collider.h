#pragma once
#include <memory>

namespace physics
{
class Scene;


class Collider
{
public:
    void add_to_scene(Scene* scene);

	static std::shared_ptr<Collider> create();

  private:

	Scene* owning_scene = nullptr;
};
}
