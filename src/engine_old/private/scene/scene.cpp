

#include "scene/scene.h"

#include "scene/node_primitive.h"

struct ModMatrix
{
    glm::mat4 a;
};
Scene::Scene()
{
}

void Scene::tick(const double delta_second)
{
    BEGIN_NAMED_RECORD(TICK_WORLD);
    for (const auto& component : scene_nodes)
        component->tick(delta_second);
}