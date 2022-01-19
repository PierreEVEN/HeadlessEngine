#include "physics/physic_scene.h"

#include <bullet/btBulletDynamicsCommon.h>

namespace physics
{

class BulletScene : public Scene
{
  public:
    BulletScene() : Scene()
    {
        collision_config     = new btDefaultCollisionConfiguration();
        broad_phase          = new btDbvtBroadphase();
        solver               = new btSequentialImpulseConstraintSolver();
        collision_dispatcher = new btCollisionDispatcher(collision_config);
        physic_scene                = new btDiscreteDynamicsWorld(collision_dispatcher, broad_phase, solver, collision_config);
    }
    ~BulletScene() override
    {
        delete physic_scene;
        delete solver;
        delete broad_phase;
        delete collision_dispatcher;
        delete collision_config;
    }

  private:
    void step() override
    {
        physic_scene->stepSimulation();
    }
    void add_collider(Collider* collider) override
    {
        physic_scene->addCollisionObject(collider);
    }
    void remove_collider(Collider* collider) override
    {
        physic_scene->removeCollisionObject(collider);
    }

public:
private:
    btBroadphaseInterface*    broad_phase;
    btDispatcher*             collision_dispatcher;
    btConstraintSolver*       solver;
    btCollisionConfiguration* collision_config;
    btDynamicsWorld*          physic_scene;
};

std::shared_ptr<Scene> Scene::create()
{
    return std::make_shared<BulletScene>();
}
} // namespace physics
