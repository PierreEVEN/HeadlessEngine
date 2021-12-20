#pragma once
#include "ecs_type.h"

#include "method_runner.h"

#include <algorithm>

namespace gfx
{
class CommandBuffer;
}

namespace ecs
{
// Concept that test if given function is implemented
template <typename T> concept has_pre_render_method = requires(T& t)
{
    t.pre_render();
};
template <typename T> concept has_render_method = requires(T& t)
{
    t.render(nullptr);
};
template <typename T> concept has_tick_method = requires(T& t)
{
    t.tick();
};

using ComponentTypeID   = EcsID;
using ComponentDataType = uint8_t;

class IComponent
{
  public:
    IComponent() = default;
    virtual ~IComponent()
    {
        delete pre_render_runner;
        delete render_runner;
        delete tick_runner;
    }

    virtual void                 component_destroy(ComponentDataType* data) const                                = 0;
    virtual void                 component_move(ComponentDataType* source, ComponentDataType* destination) const = 0;
    [[nodiscard]] virtual size_t type_size() const                                                               = 0;

    void resize_component_memory(size_t new_size, struct ActorVariant* variant, size_t component_type_index) const;

    // Different runner for each component's method
    IMethodRunner<>*                    pre_render_runner = nullptr;
    IMethodRunner<gfx::CommandBuffer*>* render_runner     = nullptr;
    IMethodRunner<>*                    tick_runner       = nullptr;
};

template <typename Component_T> class TComponent final : public IComponent
{
  public:
    TComponent()
    {
        // If tested component has a prerender/render/tick function, add a runner for this function
        if constexpr (has_pre_render_method<Component_T>)
            pre_render_runner = new TMethodRunner<Component_T>(&Component_T::pre_render);

        if constexpr (has_render_method<Component_T>)
            render_runner = new TMethodRunner<Component_T, gfx::CommandBuffer*>(&Component_T::render);

        if constexpr (has_tick_method<Component_T>)
            tick_runner = new TMethodRunner<Component_T>(&Component_T::tick);
    }

    void component_destroy(ComponentDataType* component_ptr) const override
    {
        Component_T* fixed_component_ptr = std::launder(reinterpret_cast<Component_T*>(component_ptr));
        fixed_component_ptr->~Component_T();
    }

    void component_move(ComponentDataType* source, ComponentDataType* destination) const override
    {
        new (&destination[0]) Component_T(std::move(*reinterpret_cast<Component_T*>(source)));
    }

    [[nodiscard]] static ComponentTypeID get_type_id()
    {
        return TypeIdGenerator<IComponent>::get<Component_T>();
    }
    [[nodiscard]] size_t type_size() const override
    {
        return sizeof(Component_T);
    }
};
} // namespace ecs