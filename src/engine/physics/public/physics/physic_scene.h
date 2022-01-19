#pragma once
#include "collider.h"

#include <memory>

namespace physics
{

class Scene
{
  public:
    virtual ~Scene() = default;
    static std::shared_ptr<Scene> create();

    virtual void add_collider(Collider* collider)    = 0;
    virtual void remove_collider(Collider* collider) = 0;
    virtual void step()                              = 0;
    
  protected:
    Scene() = default;

  private:
};
} // namespace physics