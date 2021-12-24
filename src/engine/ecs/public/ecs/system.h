#pragma once
#include "actor_meta_data.h"
#include "component.h"

#include <cpputils/logger.hpp>
#include <functional>
#include <vector>

namespace ecs
{
template <typename... Component_T> struct TSystemIterator
{
    TSystemIterator(size_t in_index, std::tuple<Component_T*...> in_components = std::tuple<Component_T*...>{}) : index(in_index), storage(in_components)
    {
    }

    using type = std::tuple<Component_T&...>;

    type operator++()
    {
        return ++index, operator*();
    }

    [[nodiscard]] type operator*()
    {
        return std::apply(
            [ptr_index = index](Component_T*... from_ptr)
            {
                return std::forward_as_tuple(*(from_ptr + ptr_index)...);
            },
            storage);
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

    size_t                      index;
    std::tuple<Component_T*...> storage;

};

template <typename... Component_T> class TSystemIterable
{
  public:
    using iterator = TSystemIterator<Component_T...>;

    TSystemIterable(ActorVariant* variant)
        : from(0, build_base<0, Component_T...>(variant)),
          to(variant->linked_actors.size())
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
        for (const auto& component : variant->variant_specification)
        {
            if (component == TComponent<CurrentComp_T>::get_type_id())
            {
                first_component_ptr = reinterpret_cast<CurrentComp_T*>(variant->component_data[index]);
                break;
            }
            index++;
        }

        if (!first_component_ptr)
            LOG_FATAL("system was run on an inconsistant variant : requiring %s", typeid(CurrentComp_T).name());

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
    virtual void                                       tick(ActorVariant* variant) = 0;
    [[nodiscard]] virtual std::vector<ComponentTypeID> get_key() const             = 0;
};

template <typename... Component_T> class TSystem final : public ISystem
{
  public:
    [[nodiscard]] std::vector<ComponentTypeID> get_key() const override
    {
        std::vector<ComponentTypeID> component_type = {{TComponent<Component_T>::get_type_id()...}};
        std::ranges::sort(component_type);
        return component_type;
    }

    void tick(ActorVariant* variant) override
    {
        if (tick_func)
            tick_func(TSystemIterable<Component_T...>(variant));
    }

    TSystem(std::function<void(TSystemIterable<Component_T...>)> tick_function) : tick_func(std::move(tick_function))
    {
    }

    std::function<void(TSystemIterable<Component_T...>)> tick_func;
};

class SystemFactory
{
  public:
    template <typename... Component_T, typename Lambda> void tick(Lambda callback)
    {
        ticks.emplace_back(new TSystem<Component_T...>(callback));
    }

    template <typename... Component_T, typename Lambda> void pre_render(Lambda callback)
    {
        pre_renders.emplace_back(new TSystem<Component_T...>(callback));
    }

    template <typename... Component_T, typename Lambda> void render(Lambda callback)
    {
        renders.emplace_back(new TSystem<Component_T...>(callback));
    }

    void execute_tick() const;

  private:
    std::vector<std::unique_ptr<ISystem>> ticks;
    std::vector<std::unique_ptr<ISystem>> pre_renders;
    std::vector<std::unique_ptr<ISystem>> renders;
};
} // namespace ecs
