#include "ecs_test.h"

struct Position
{
    float x;
    float y;
};

struct Velocity
{
    float x;
    float y;
};

struct Randomness
{
    float a;
};

void test_func()
{

    ECS ecs;
    ecs.RegisterComponent<Position>();
    ecs.RegisterComponent<Velocity>();
    ecs.RegisterComponent<Randomness>();

    Entity ent1(ecs);
    ent1.Add<Position>({1, 2});
    ent1.Add<Velocity>({.5f, .5f});
    ent1.Add<Randomness>({.25f});

    Entity ent2(ecs);
    ent2.Add<Position>({5, 24});
    ent2.Add<Randomness>({0.8f});
}