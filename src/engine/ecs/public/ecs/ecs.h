#pragma once
#include "ecs/system.h"
#include "actor_meta_data.h"
#include "component.h"
#include "ecs_type.h"

#include <cpputils/logger.hpp>
#include <unordered_map>

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
  public:
    static ECS& get();
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

    void tick();
    void pre_render();
    void render(gfx::CommandBuffer* command_buffer);

  private:
    friend struct ActorVariant;

    ECS() = default;

    std::unordered_map<ComponentTypeID, IComponent*> component_registry;
    std::unordered_map<ActorID, ActorMetaData>       actor_registry;
    std::vector<ActorVariant*>                       variant_registry;

    ActorID       last_actor_id = 0;
    SystemFactory system_factory;
};

template <class Component_T, typename... CtorArgs_T> Component_T* ECS::add_component(const ActorID& to_actor, CtorArgs_T&&... args)
{
    Component_T* component_memory = nullptr;

    if (!is_component_type_registered<Component_T>())
    {
        LOG_ERROR("component type %s is not registered. Please call ECS::get().register_component<%s>(); before", typeid(Component_T).name(), typeid(Component_T).name());
        return component_memory;
    }

    // Retrieve initial context and type infos
    const ComponentTypeID added_component_type_id = TComponent<Component_T>::get_type_id();
    ActorMetaData*        actor_data              = &actor_registry[to_actor];

    // Actor doesn't have components
    if (!actor_data->variant)
    {
        // 1) Find an existing variant with given specs or create a new one
        std::vector new_variant_spec = {added_component_type_id};
        auto*             final_variant    = find_variant(new_variant_spec);

        final_variant->add_actor(actor_data);
        component_memory = new (final_variant->get_last_element_memory(added_component_type_id)) Component_T(std::forward<CtorArgs_T>(args)...);
    }
    else // We need to find an existing variant or to create a new one for the given actor
    {
        // Add component to the existing spec
        std::vector<ComponentTypeID> new_specifications = actor_data->variant->get_specification();
        new_specifications.emplace_back(added_component_type_id);
        std::ranges::sort(new_specifications); // don't forget too always sort variant to avoid duplications

        auto* final_variant = find_variant(new_specifications); // find or create variant with given specs

        final_variant->move_actor_from(actor_data, actor_data->variant);
        component_memory = new (final_variant->get_last_element_memory(added_component_type_id)) Component_T(std::forward<CtorArgs_T>(args)...);
    }
    return component_memory;
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
