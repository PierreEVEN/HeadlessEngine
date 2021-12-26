#pragma once
#include "actor_meta_data.h"
#include "component.h"
#include "ecs/SystemTick.h"
#include "ecs_type.h"

#include <cpputils/eventmanager.hpp>
#include <cpputils/logger.hpp>
#include <types/robin_hood_map.h>

namespace gfx
{
class View;
}

DECLARE_DELEGATE_MULTICAST(OnTickDelegate);
DECLARE_DELEGATE_MULTICAST(OnPreRenderDelegate, gfx::View*);
DECLARE_DELEGATE_MULTICAST(OnRenderDelegate, gfx::View*);

namespace ecs
{
void ecs_test();

template <typename T>
concept has_add_systems_function = requires(T& t)
{
    T::add_systems(nullptr);
};

class ECS final
{
    friend ECS& singleton();

  public:
    ~ECS();

    template <class Component_T> bool is_component_type_registered()
    {
        return component_registry.contains(TComponent<Component_T>::get_type_id());
    }

    template <class Component_T> void register_component_type()
    {
        if (is_component_type_registered<Component_T>())
            return;
        component_registry[TComponent<Component_T>::get_type_id()] = new TComponent<Component_T>();

        if constexpr (has_add_systems_function<Component_T>)
            Component_T::add_systems(&system_factory);
    }

    void                                                              add_empty_actor(const ActorID actor_instance_id);
    void                                                              remove_actor(const ActorID& removed_actor);
    template <class Component_T, typename... CtorArgs_T> Component_T* add_component(const ActorID& to_actor, CtorArgs_T&&... args);
    template <class Component_T> void                                 remove_component(const ActorID& from_actor);
    [[nodiscard]] ActorVariant*                                       find_variant(std::vector<ComponentTypeID>& variant_spec);
    [[nodiscard]] ActorID                                             make_new_actor_id();

    [[nodiscard]] const std::vector<ActorVariant*>& get_variants() const
    {
        return variant_registry;
    }

    [[nodiscard]] IComponent* get_component_type(ComponentTypeID type_id)
    {
        return component_registry[type_id];
    }

    [[nodiscard]] SystemFactory* get_system_factory()
    {
        return &system_factory;
    }

    [[nodiscard]] size_t count_actors() const
    {
        return actor_registry.size();
    }

    void tick();
    void pre_render(gfx::View* view);
    void render(gfx::View* view);

    OnTickDelegate on_tick;
    OnPreRenderDelegate on_pre_render;
    OnRenderDelegate on_render;

  private:
    friend struct ActorVariant;

    ECS() = default;

    robin_hood::unordered_map<ComponentTypeID, IComponent*> component_registry;
    robin_hood::unordered_map<ActorID, ActorMetaData>       actor_registry;
    robin_hood::unordered_map<ActorID, uint32_t>            actor_links;
    std::vector<ActorVariant*>                              variant_registry;

    ActorID       last_actor_id = 0;
    SystemFactory system_factory;
};

ECS& singleton();

template <class Component_T, typename... CtorArgs_T> Component_T* ECS::add_component(const ActorID& to_actor, CtorArgs_T&&... args)
{
    if (!is_component_type_registered<Component_T>())
        register_component_type<Component_T>();

    // Retrieve initial context and type infos
    const ComponentTypeID added_component_type_id = TComponent<Component_T>::get_type_id();
    ActorMetaData*        actor_data              = &actor_registry[to_actor];

    // Actor doesn't have components
    if (!actor_data->variant)
    {
        // 1) Find an existing variant with given specs or create a new one
        std::vector new_variant_spec = {added_component_type_id};
        auto*       final_variant    = find_variant(new_variant_spec);

        final_variant->add_actor(actor_data);
        return new (final_variant->get_last_element_memory(added_component_type_id)) Component_T(std::forward<CtorArgs_T>(args)...);
    }
    else // We need to find an existing variant or to create a new one for the given actor
    {
        // Add component to the existing spec
        std::vector<ComponentTypeID> new_specifications = actor_data->variant->get_specification();
        new_specifications.emplace_back(added_component_type_id);
        std::ranges::sort(new_specifications); // don't forget too always sort variant to avoid duplications

        auto* final_variant = find_variant(new_specifications); // find or create variant with given specs

        final_variant->move_actor_from(actor_data, actor_data->variant);
        return new (final_variant->get_last_element_memory(added_component_type_id)) Component_T(std::forward<CtorArgs_T>(args)...);
    }
}

template <class Component_T> void ECS::remove_component(const ActorID& from_actor)
{
    // The given component is not registered
    if (!is_component_type_registered<Component_T>())
        return;

    // The given actor does not exists
    if (!actor_registry.contains(from_actor))
        return;

    const ComponentTypeID compTypeId = TComponent<Component_T>::get_type_id();
    ActorMetaData*        actor_data = &actor_registry[from_actor];

    // The component list is already empty
    if (!actor_data->variant)
        return;

    // 1) Find an existing variant with given specs or create a new one
    std::vector<ComponentTypeID> final_variant_specs = actor_data->variant->get_specification();
    final_variant_specs.erase(std::ranges::remove(final_variant_specs, compTypeId).begin(), final_variant_specs.end());
    std::ranges::sort(final_variant_specs);

    if (final_variant_specs.empty())
    {
        actor_data->variant->remove_actor(actor_data);
    }
    else
    {
        ActorVariant* final_variant = find_variant(final_variant_specs);
        final_variant->move_actor_from(actor_data, actor_data->variant);
    }
}
} // namespace ecs
