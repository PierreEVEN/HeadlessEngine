#pragma once
#include "actor_meta_data.h"
#include "ComponentHelper.h"

#include <cpputils/logger.hpp>
#include <functional>
#include <vector>

namespace ecs
{
template <typename... Component_T> struct TSystemIterator
{
    TSystemIterator(size_t in_index, ActorID* actor_it = nullptr, std::tuple<Component_T*...> in_components = std::tuple<Component_T*...>{}) : actor_iterator(actor_it), index(in_index), storage(in_components)
    {
    }

    using type = std::tuple<ActorID*, Component_T&...>;

    type operator++()
    {
        return ++index, operator*();
    }

    [[nodiscard]] type operator*()
    {
        return std::tuple_cat(std::make_tuple<ActorID*>(&actor_iterator[index]), std::apply(
                                                                                     [ptr_index = index](Component_T*... from_ptr)
                                                                                     {
                                                                                         return std::forward_as_tuple(from_ptr[ptr_index]...);
                                                                                     },
                                                                                     storage));
    }

    template <typename... Other_T> [[nodiscard]] bool operator==(const TSystemIterator<Other_T...>& other) const
    {
        return this->index == other.index;
    }

    template <typename... Other_T> [[nodiscard]] bool operator!=(const TSystemIterator<Other_T...>& other) const
    {
        return !(*this == other);
    }

  private:
    ActorID*                    actor_iterator;
    size_t                      index;
    std::tuple<Component_T*...> storage;
};

template <typename... Component_T> class TSystemIterable
{
  public:
    using iterator = TSystemIterator<Component_T...>;

    TSystemIterable(ActorVariant* variant) : from(0, variant->linked_actors.data(), build_base<0, Component_T...>(variant)), to(variant->linked_actors.size())
    {
    }

    iterator begin()
    {
        return from;
    }

    iterator end()
    {
        return to;
    }

  private:
    iterator from;
    iterator to;

    template <typename CurrentComp_T> CurrentComp_T* get_first_ptr(ActorVariant* variant)
    {
        CurrentComp_T* first_component_ptr = nullptr;

        size_t index = 0;
        for (const auto& component : variant->components)
        {
            if (component.type_id == TComponentHelper<CurrentComp_T>::get_type_id())
            {
                first_component_ptr = reinterpret_cast<CurrentComp_T*>(variant->components[index].component_data.data());
                break;
            }
            index++;
        }

        if (!first_component_ptr)
            LOG_FATAL("system was run on an inconsistant family : requiring %s", typeid(CurrentComp_T).name());

        return first_component_ptr;
    }

    template <size_t CompIndex_T, typename CurrentComp_T, typename... Other_T> std::enable_if_t<CompIndex_T != sizeof...(Component_T) - 1, std::tuple<CurrentComp_T*, Other_T*...>> build_base(ActorVariant* variant)
    {
        return std::tuple_cat(std::make_tuple<CurrentComp_T*>(get_first_ptr<CurrentComp_T>(variant)), build_base<CompIndex_T + 1, Other_T...>(variant));
    }

    template <size_t CompIndex_T, typename CurrentComp_T> std::enable_if_t<CompIndex_T == sizeof...(Component_T) - 1, std::tuple<CurrentComp_T*>> build_base([[maybe_unused]] ActorVariant* variant)
    {
        return std::make_tuple<CurrentComp_T*>(get_first_ptr<CurrentComp_T>(variant));
    }
};

class ISystem
{
  public:
    virtual void tick([[maybe_unused]] ActorVariant* variant)
    {
    }
    virtual void pre_render([[maybe_unused]] ActorVariant* variant, [[maybe_unused]] gfx::View* view)
    {
    }
    virtual void render([[maybe_unused]] ActorVariant* variant, [[maybe_unused]] gfx::View* view)
    {
    }
    [[nodiscard]] virtual std::vector<ComponentTypeID> get_key() const = 0;
};

template <typename... Component_T> class TSystemTick final : public ISystem
{
  public:
    using TickFunction = std::function<void(TSystemIterable<Component_T...>)>;

    TSystemTick(TickFunction tick_function) : tick_func(std::move(tick_function))
    {
    }

    [[nodiscard]] std::vector<ComponentTypeID> get_key() const override
    {
        std::vector<ComponentTypeID> component_type = {{TComponentHelper<Component_T>::get_type_id()...}};
        std::ranges::sort(component_type);
        return component_type;
    }

    void tick(ActorVariant* variant) override
    {
        if (tick_func)
            tick_func(TSystemIterable<Component_T...>(variant));
    }

    TickFunction tick_func;
};
template <typename... Component_T> class TSystemRender final : public ISystem
{
  public:
    using RenderFunction = std::function<void(TSystemIterable<Component_T...>, gfx::View*)>;

    TSystemRender(RenderFunction tick_function) : render_func(std::move(tick_function))
    {
    }

    [[nodiscard]] std::vector<ComponentTypeID> get_key() const override
    {
        std::vector<ComponentTypeID> component_type = {{TComponentHelper<Component_T>::get_type_id()...}};
        std::ranges::sort(component_type);
        return component_type;
    }

    void pre_render(ActorVariant* variant, gfx::View* view) override
    {
        if (render_func)
            render_func(TSystemIterable<Component_T...>(variant), view);
    }

    void render(ActorVariant* variant, gfx::View* view) override
    {
        if (render_func)
            render_func(TSystemIterable<Component_T...>(variant), view);
    }

    RenderFunction render_func;
};

class SystemFactory
{
  public:
    template <typename... Component_T, typename Lambda> void tick(Lambda callback)
    {
        ticks.emplace_back(new TSystemTick<Component_T...>(callback));
    }

    template <typename... Component_T, typename Lambda> void pre_render(Lambda callback)
    {
        pre_renders.emplace_back(new TSystemRender<Component_T...>(callback));
    }

    template <typename... Component_T, typename Lambda> void render(Lambda callback)
    {
        renders.emplace_back(new TSystemRender<Component_T...>(callback));
    }

    void execute_tick() const;
    void execute_pre_render(gfx::View* view) const;
    void execute_render(gfx::View* view) const;

  private:
    std::vector<std::unique_ptr<ISystem>> ticks;
    std::vector<std::unique_ptr<ISystem>> pre_renders;
    std::vector<std::unique_ptr<ISystem>> renders;
};
} // namespace ecs
