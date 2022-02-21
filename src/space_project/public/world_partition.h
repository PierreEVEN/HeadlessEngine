#pragma once

#include "ecs/ecs.h"
#include "ecs/ecs_new.h"
#include "ecs/object.h"
#include "reflection/class.h"

#include <glm/glm.hpp>

class ViewPoint
{
};

class World final
{
  public:
    int test;

  private:
    ecs::ECS world_content;
};

class WorldPartition final
{
  public:
    WorldPartition();
    virtual ~WorldPartition() = default;

    void include(WorldPartition& other);
    void render(const ViewPoint& view_point);
    
  private:
    glm::dvec3 velocity;
    glm::dvec3 aabb_center;
    glm::dvec3 aabb_extent;
};

///////// @todo

/***
 * USE CASE EXAMPLES
 */
class Transform : public ComponentBase
{
    COMPONENT_BODY;
  public:

    void translate()
    {
        // propagate and update translation
        for (auto& child : children)
            child->translate();
    }

    virtual void on_move(Ecs_New* new_context) override
    {
        for (auto& child : children)
            child->object().move_to(new_context);
    }

  private:

    PROPERTY()
    Component<Transform> parent;

    PROPERTY(Transient)
    std::vector<Component<Transform>> children;
};

class Mesh : public ComponentBase
{
    COMPONENT_BODY;

  public:
    Mesh()
    {
    }

    void tick() override
    {
        if (get_component<Transform>())
            get_component<Transform>()->translate();

        if (gameplay_manager.get_component<Transform>())
            gameplay_manager.get_component<Transform>()->translate();
    }

  private:
    PROPERTY()
    Component<Mesh> test_next;
    Object          gameplay_manager;
};

void test()
{
    Ecs_New ctx0;
    Ecs_New ctx1;

    Object obj_a = ctx0.new_object();
    obj_a.add_component<Mesh>();
    Component<Mesh> mesh = obj_a.get_component<Mesh>();
    mesh.object().move_to(&ctx1);
    mesh.object().remove_component<Mesh>();


    Mesh* m = new Mesh();
    Transform* tr = new Transform();

    if (cast<Transform>(m))
    {
        
    }
    Class* cl = m->get_class();
    (void)cl;
    (void)tr;
}

// RULES :
//
// Use Component<T> and Object to point to components and entities
//
// Object : when move_to is called : call on_move to each components