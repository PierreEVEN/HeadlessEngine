#pragma once
#include "ecs/common.h"
#include "ecs_type.h"

#include "method_runner.h"

#include <algorithm>

namespace ecs
{

using ComponentTypeID   = EcsID;
using ComponentDataType = uint8_t;
class ECS;

class IComponentHelper
{
  public:
    IComponentHelper() = default;
    virtual ~IComponentHelper()
    {
    }

    virtual void                 component_destroy(ComponentDataType* data) const                                = 0;
    virtual void                 component_move(ComponentDataType* source, ComponentDataType* destination) const = 0;
    [[nodiscard]] virtual size_t type_size() const                                                               = 0;

    // Different runner for each component's method
    std::unique_ptr<IMethodRunner<gfx::View*>>                      pre_render_runner = nullptr;
    std::unique_ptr<IMethodRunner<gfx::View*, gfx::CommandBuffer*>> render_runner     = nullptr;
    std::unique_ptr<IMethodRunner<>>                                tick_runner       = nullptr;
    std::unique_ptr<IMethodRunner<ECS*, ECS*>>                      move_runner       = nullptr;
};

template <typename Component_T> class TComponentHelper final : public IComponentHelper
{
  public:
    TComponentHelper()
    {
        if constexpr (has_tick_method<Component_T>)
            tick_runner = std::make_unique<TMethodRunner<Component_T>>(&Component_T::tick);
        if constexpr (has_pre_render_method<Component_T>)
            pre_render_runner = std::make_unique<TMethodRunner<Component_T, gfx::View*>>(&Component_T::pre_render);
        if constexpr (has_render_method<Component_T>)
            render_runner = std::make_unique<TMethodRunner<Component_T, gfx::View*, gfx::CommandBuffer*>>(&Component_T::render);
        if constexpr (has_move_method<Component_T>)
            move_runner = std::make_unique<TMethodRunner<Component_T, ECS*, ECS*>>(&Component_T::move);
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
        return TypeIdGenerator<IComponentHelper>::get<Component_T>();
    }

    [[nodiscard]] size_t type_size() const override
    {
        return sizeof(Component_T);
    }
};
} // namespace ecs