
#include "physics/collider.h"

#include "physics/physic_scene.h"

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>

namespace physics
{
void Collider::add_to_scene(Scene* scene)
{
    if (owning_scene && owning_scene != scene)
    {
        owning_scene->remove_collider(this);
    }
    owning_scene = scene;

    if (owning_scene)
        scene->add_collider(this);
}

std::shared_ptr<Collider> Collider::create()
{
    return std::make_shared<BulletCollider>();
}
} // namespace physics
