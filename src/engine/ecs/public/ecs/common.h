#pragma once
#include <cpputils/eventmanager.hpp>

namespace gfx
{
class View;
class CommandBuffer;
}

namespace ecs
{
DECLARE_DELEGATE_MULTICAST(OnTickDelegate);
DECLARE_DELEGATE_MULTICAST(OnPreRenderDelegate, gfx::View*);
DECLARE_DELEGATE_MULTICAST(OnRenderDelegate, gfx::View*, gfx::CommandBuffer*);

// Concept that test if given function is implemented
template <typename T>
concept has_add_systems_function = requires(T& t)
{
    T::add_systems(nullptr);
};
template <typename T>
concept has_pre_render_method = requires(T& t)
{
    t.pre_render();
};
template <typename T>
concept has_render_method = requires(T& t)
{
    t.render(nullptr, nullptr);
};
template <typename T>
concept has_tick_method = requires(T& t)
{
    t.tick();
};
template <typename T>
concept has_move_method = requires(T& t)
{
    t.move_to(nullptr, nullptr);
};
}
