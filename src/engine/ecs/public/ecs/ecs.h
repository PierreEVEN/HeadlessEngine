#pragma once
#include "gfx/command_buffer.h"

#include <functional>

namespace ecs
{

class Component
{
  protected:
    virtual std::function<void(Component*, gfx::CommandBuffer*)> render()
    {
        return nullptr;
    }
    virtual std::function<void(Component*)> pre_render()
    {
        return nullptr;
    }
    virtual std::function<void(Component*)> tick()
    {
        return nullptr;
    }
    virtual std::function<void(Component*)> physic()
    {
        return nullptr;
    }
};

template <typename T, typename E, typename... Args> struct component_has_method
{
    template <typename U, E (U::*)(Args...) const> struct SfinaeSTR
    {
    };

    template <typename U> static char _test(SfinaeSTR<U, &U::E>*);
    template <typename U> static int  _test(...);
    static const bool                 value = sizeof(_test<T>(0)) == sizeof(char);
};


class TestComponent : public Component
{
  public:
  protected:
    void render(gfx::CommandBuffer* buffer)
    {
        test_data = 5;

        gfx::Mesh* mesh = nullptr;
        buffer->draw_mesh(mesh, nullptr, {}, test_data);
    }

  private:
    int test_data;
};


class ComponentClass
{
    using test_func_ptr = void (TestComponent::*)();

    void test()
    {
        test_func_ptr test_func = {};

        TestComponent* comp = new TestComponent();

        (comp->*test_func)();
    }



};



} // namespace ecs
