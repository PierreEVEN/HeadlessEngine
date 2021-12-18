#pragma once
#include "gfx/command_buffer.h"

#include <cpputils/logger.hpp>
#include <functional>

namespace ecs
{
// IRunner : component-independent interface
template <typename... Args_T> class IExpressionRunner
{
  public:
    virtual void execute(void* instances, size_t count, Args_T&&... args) = 0;
};

// TRunner : execute a given method on a vector of components
template <typename Component_T, typename... Args_T> class TExpressionRunner : public IExpressionRunner<Args_T...>
{
  public:
    using Method = void (Component_T::*)(Args_T...);

    TExpressionRunner(const Method render_func) : method(render_func)
    {
    }

private:
    void execute(void* start_ptr, size_t count, Args_T&&... args) override
    {
        auto* current_component = static_cast<Component_T*>(start_ptr);

        for (auto * end_component = static_cast<Component_T*>(start_ptr) + count; current_component != end_component; current_component += 1)
            (current_component->*method)(std::forward<Args_T>(args)...);
    }

    Method method;
};

struct ComponentData
{
    // Different runner for each component's method
    IExpressionRunner<>*                    pre_render_runner = nullptr;
    IExpressionRunner<gfx::CommandBuffer*>* render_runner     = nullptr;
    IExpressionRunner<>*                    tick_runner       = nullptr;
};

// Concept that test if given function is implemented
template <typename T> concept has_pre_render_method = requires(T& t)
{
    t.prerender();
};
template <typename T> concept has_render_method = requires(T& t)
{
    t.render(nullptr);
};
template <typename T> concept has_tick_method = requires(T& t)
{
    t.tick();
};

void                                          register_component_type_internal(const ComponentData& component_data);
template <typename Component_T> ComponentData register_component()
{
    ComponentData component_data;

    // If tested component has a prerender/render/tick function, add a runner for this function
    if constexpr (has_pre_render_method<Component_T>)
        component_data.pre_render_runner = new TExpressionRunner<Component_T>(&Component_T::pre_render);
    if constexpr (has_render_method<Component_T>)
        component_data.render_runner = new TExpressionRunner<Component_T, gfx::CommandBuffer*>(&Component_T::render);
    if constexpr (has_tick_method<Component_T>)
        component_data.tick_runner = new TExpressionRunner<Component_T>(&Component_T::tick);

    register_component_type_internal(component_data);
    return component_data;
}

class MyComponent
{
  public:
    void pre_render();
    void render(gfx::CommandBuffer* command_buffer);
    int internal_val;
};

inline void ecs_test()
{
    // register different components
    auto comp_data  = register_component<MyComponent>();

    // Create an example of components
    const size_t component_count      = 10000000;
    auto*        components_start_ptr = static_cast<uint8_t*>(malloc(sizeof(MyComponent) * 10000000));
    for (size_t i = 0; i < component_count; ++i)
        new (components_start_ptr + i * sizeof(MyComponent)) MyComponent();

    // Example game loop
    do
    {
        // foreach comp class
        do
        {
            if (comp_data.pre_render_runner)
                comp_data.pre_render_runner->execute(components_start_ptr, component_count);

            if (comp_data.render_runner)
                comp_data.render_runner->execute(components_start_ptr, component_count, nullptr);

            if (comp_data.tick_runner)
                comp_data.tick_runner->execute(components_start_ptr, component_count);
        } while (false);
    } while (false);
    
    auto* current_component = reinterpret_cast<MyComponent*>(components_start_ptr);
    auto* end_component     = reinterpret_cast<MyComponent*>(components_start_ptr) + component_count;

    for (; current_component != end_component; current_component += 1)
        current_component->render(nullptr);

    free(components_start_ptr);
}

} // namespace ecs