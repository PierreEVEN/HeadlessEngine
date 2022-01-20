#pragma once
#include "actor_meta_data.h"
#include "component.h"
#include "ecs/system.h"
#include "ecs_type.h"
#include "ecs/common.h"

#include <cpputils/logger.hpp>
#include <types/robin_hood_map.h>

namespace ecs
{
class Actor;

class ECS
{
  public:
    ECS() = default;
    virtual ~ECS();

    template <class Component_T> static bool is_component_type_registered()
    {
        return component_registry.contains(TComponentHelper<Component_T>::get_type_id());
    }

    template <class Component_T> static void register_component_type()
    {
        if (is_component_type_registered<Component_T>())
            return;
        component_registry[TComponentHelper<Component_T>::get_type_id()] = new TComponentHelper<Component_T>();

        // Register systems
        if constexpr (has_add_systems_function<Component_T>)
            Component_T::add_systems(&system_factory);
    }

    std::shared_ptr<Actor> new_actor();

    [[nodiscard]] const std::vector<ActorVariant*>& get_variants() const
    {
        return variant_registry;
    }

    [[nodiscard]] IComponentHelper* get_component_type(ComponentTypeID type_id)
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

    // Propagate tick through components
    virtual void tick();
    // Propagate pre_render through components
    virtual void pre_render(gfx::View* view);
    // Propagate render through components
    virtual void render(gfx::View* view, gfx::CommandBuffer* command_buffer);

    // Event called on tick
    OnTickDelegate      on_tick;
    // Event called on pre_render
    OnPreRenderDelegate on_pre_render;
    // Event called on render
    OnRenderDelegate    on_render;

  private:
    friend class Actor;
    friend class ActorVariant;

    // Add an actor to this ECS
    void register_actor(const ActorID& actor);
    // Remove an actor from this ECS
    void remove_actor(const ActorID& removed_actor);
    // Move an actor to an other ECS (will call on_move in each components)
    static void move_actor(ActorID actor, ECS* old_context, ECS* new_context);
    // Duplicate an existing actor
    ActorID     duplicate_actor(ActorID actor);
    // Create a global actor UID. This id is unique with all ECS.
    [[nodiscard]] static ActorID make_new_actor_id();

    // Add or remove components from an actor
    template <class Component_T, typename... CtorArgs_T> Component_T* add_component(const ActorID& to_actor, CtorArgs_T&&... args);
    template <class Component_T> void                                 remove_component(const ActorID& from_actor);
    
    [[nodiscard]] ActorVariant* find_variant(std::vector<ComponentTypeID>& variant_spec);

    // A list of existing component type that are in use
    inline static robin_hood::unordered_map<ComponentTypeID, IComponentHelper*> component_registry;

    // All the data relative to current ECS content
    robin_hood::unordered_map<ActorID, ActorMetaData> actor_registry;
    std::vector<ActorVariant*>                        variant_registry;
    robin_hood::unordered_map<ActorID, uint32_t>      actor_links;

    SystemFactory system_factory;
};

template <class Component_T, typename... CtorArgs_T> Component_T* ECS::add_component(const ActorID& to_actor, CtorArgs_T&&... args)
{
    if (!is_component_type_registered<Component_T>())
        register_component_type<Component_T>();

    // Retrieve initial context and type infos
    const ComponentTypeID new_component_type_id = TComponentHelper<Component_T>::get_type_id();
    ActorMetaData*        actor_data              = &actor_registry[to_actor];

    // Actor doesn't have components
    if (!actor_data->variant)
    {
        // 1) Find an existing variant with given specs or create a new one
        std::vector new_variant_spec = {new_component_type_id};
        auto*       new_variant    = find_variant(new_variant_spec);

        // Create and register actor
        new_variant->add_actor(actor_data);
        return new (new_variant->get_last_element_memory(new_component_type_id)) Component_T(std::forward<CtorArgs_T>(args)...);
    }
    else // We need to find an existing variant or to create a new one for the given actor
    {
        // Retrieve existing variant
        std::vector<ComponentTypeID> new_specifications = actor_data->variant->get_specification();
        // Add the newly added component
        new_specifications.emplace_back(new_component_type_id);
        auto* final_variant = find_variant(new_specifications); // find or create variant with given specs

        final_variant->copy_to_this_variant(actor_data, actor_data->variant);
        return new (final_variant->get_last_element_memory(new_component_type_id)) Component_T(std::forward<CtorArgs_T>(args)...);
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

    const ComponentTypeID compTypeId = TComponentHelper<Component_T>::get_type_id();
    ActorMetaData*        actor_data = &actor_registry[from_actor];

    // The component list is already empty
    if (!actor_data->variant)
        return;

    // 1) Find an existing variant with given specs or create a new one
    std::vector<ComponentTypeID> final_variant_specs = actor_data->variant->get_specification();
    final_variant_specs.erase(std::ranges::remove(final_variant_specs, compTypeId).begin(), final_variant_specs.end());

    if (final_variant_specs.empty())
    {
        actor_data->variant->remove_actor(actor_data);
    }
    else
    {
        ActorVariant* final_variant = find_variant(final_variant_specs);
        final_variant->copy_to_this_variant(actor_data, actor_data->variant);
    }
}
} // namespace ecs
