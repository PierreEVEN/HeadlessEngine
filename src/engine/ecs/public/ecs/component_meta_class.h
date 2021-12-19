#pragma once
#include "method_runner.h"

namespace gfx
{
class CommandBuffer;
}

namespace ecs
{

struct ComponentData
{
    // Different runner for each component's method
    IMethodRunner<>*                    pre_render_runner = nullptr;
    IMethodRunner<gfx::CommandBuffer*>* render_runner     = nullptr;
    IMethodRunner<>*                    tick_runner       = nullptr;
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
        component_data.pre_render_runner = new TMethodRunner<Component_T>(&Component_T::pre_render);

    if constexpr (has_render_method<Component_T>)
        component_data.render_runner = new TMethodRunner<Component_T, gfx::CommandBuffer*>(&Component_T::render);

    if constexpr (has_tick_method<Component_T>)
        component_data.tick_runner = new TMethodRunner<Component_T>(&Component_T::tick);

    register_component_type_internal(component_data);
    return component_data;
}

}
