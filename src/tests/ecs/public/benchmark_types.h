#pragma once

#include "ecs/system.h"

#define BENCH_ENTITIES 10000000

class FirstComponent
{
  public:
    FirstComponent(float test_value) : value(test_value)
    {
    }

    FirstComponent() : value(14) {}

    __declspec(noinline) void tick()
    {
        value++;
    }

    __declspec(noinline) static void add_systems(ecs::SystemFactory* factory)
    {
        factory->tick<FirstComponent>(
            [](ecs::TSystemIterable<FirstComponent> iterator)
            {
                for (auto [entity, comp1] : iterator)
                {
                    comp1.value = static_cast<float>(*entity);
                }
            });
    }

    float value;
};

class SecondComponent
{
  public:
    SecondComponent(float test_value) : value(test_value)
    {
    }

    void tick()
    {
        value--;
    }

    float value;
};