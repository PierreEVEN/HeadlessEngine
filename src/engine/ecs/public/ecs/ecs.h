#pragma once
#include "actor_meta_data.h"
#include "component.h"
#include "ecs_type.h"

#include <cpputils/logger.hpp>
#include <unordered_map>

#include "ecs/system.h"


namespace ecs
{
void ecs_test();

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
    }

    void                                                              add_empty_actor(const ActorID actor_instance_id);
    void                                                              remove_actor(const ActorID& removed_actor);
    template <class Component_T, typename... CtorArgs_T> Component_T* add_component(const ActorID& to_actor, CtorArgs_T&&... args);
    template <class Component_T, typename... CtorArgs_T> void         remove_component(const ActorID& from_actor);
    [[nodiscard]] ActorVariant*                                       find_variant(const std::vector<ComponentTypeID>& variant_spec);
    [[nodiscard]] ActorID                                             make_new_actor_id();

    [[nodiscard]] const std::vector<ActorVariant*>& get_variants() const
    {
        return variant_registry;
    }

    [[nodiscard]] IComponent* get_component_type(ComponentTypeID type_id)
    {
        return component_registry[type_id];
    }

  private:
    ECS() = default;

    std::unordered_map<ComponentTypeID, IComponent*> component_registry;
    std::unordered_map<ActorID, ActorMetaData>       actor_meta_data;
    std::vector<ActorVariant*>                       variant_registry;
    ActorID                                          last_actor_id = 0;
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
    const size_t          type_size               = component_registry[added_component_type_id]->type_size();
    ActorMetaData&        actor_data              = actor_meta_data[to_actor];
    ActorVariant*         initial_variant         = actor_data.variant;
    ActorVariant*         final_variant           = nullptr;

    // Actor doesn't have components
    if (!initial_variant)
    {
        // 1) Find an existing variant with given specs or create a new one
        std::vector<ComponentTypeID> new_variant_spec = {added_component_type_id};
        const IComponent*            component_type   = component_registry[added_component_type_id];
        final_variant                                 = find_variant(new_variant_spec);

        // 2) Eventually resize the component's data (there is one component at this moment)
        const std::size_t actual_data_size = final_variant->linked_actors.size() * type_size;
        component_type->resize_component_memory(actual_data_size + type_size, final_variant, 0);

        // 3) When memory is ready, call constructor
        component_memory = new (&final_variant->component_data[0][actual_data_size]) Component_T(std::forward<CtorArgs_T>(args)...);
    }
    else // We need to find an existing variant or to create a new one for the given actor
    {
        // Check if the current variant already contains the specified component
        if (std::ranges::find(initial_variant->variant_specification, added_component_type_id) != initial_variant->variant_specification.end())
        {
            // Cannot support multiple components of the same type within the same actor
            LOG_ERROR("actor already contains a component of type %s", typeid(Component_T).name());
            return nullptr;
        }

        // Add component to the existing spec
        std::vector<ComponentTypeID> new_specifications = initial_variant->variant_specification;
        new_specifications.emplace_back(added_component_type_id);
        std::ranges::sort(new_specifications); // don't forget too always sort variant to avoid duplications

        final_variant = find_variant(new_specifications); // find or create variant with given specs

        // For each component of the variant
        for (std::size_t j = 0; j < new_specifications.size(); ++j)
        {
            // 1) Resize the data of each component of the new variant to accept the data from the old one
            const ComponentTypeID& new_component_id        = new_specifications[j];
            const IComponent*      new_component_type      = component_registry[new_component_id];
            const std::size_t      new_component_type_size = new_component_type->type_size();
            const std::size_t      initial_size            = final_variant->linked_actors.size() * new_component_type_size;

            new_component_type->resize_component_memory(initial_size + new_component_type_size, final_variant, j);

            // 2) Remove existing data from the old variant and move it to the new one
            for (std::size_t i = 0; i < initial_variant->variant_specification.size(); ++i)
            {
                const ComponentTypeID& old_component_id = initial_variant->variant_specification[i];
                if (old_component_id == new_component_id)
                {
                    const IComponent*  old_comp           = component_registry[old_component_id];
                    const std::size_t& old_comp_data_size = old_comp->type_size();

                    // Remove to_actor's data from the old variant and move it to the new one
                    old_comp->component_move(&initial_variant->component_data[i][actor_data.data_index * old_comp_data_size], &final_variant->component_data[j][initial_size]);
                    old_comp->component_destroy(&initial_variant->component_data[i][actor_data.data_index * old_comp_data_size]);

                    // Go to next component (a bit dirty but it's okay)
                    goto goto_continue;
                }
            }

            // 3) If it is the new component, call the constructor memory
            component_memory = new (&final_variant->component_data[j][initial_size]) Component_T(std::forward<CtorArgs_T>(args)...);

        goto_continue:;
        }

        // Remove data, references... from the old variant and move memories to keep it coherent
        if (!initial_variant->linked_actors.empty())
        {
            // For each component of the old variant
            for (std::size_t i = 0; i < initial_variant->variant_specification.size(); ++i)
            {
                const ComponentTypeID& old_component_type_id   = initial_variant->variant_specification[i];
                const IComponent*      old_component           = component_registry[old_component_type_id];
                const size_t           old_component_data_size = old_component->type_size();

                //@TEST component destroyed twice ???
                if (old_component_type_id == added_component_type_id)
                    old_component->component_destroy(&initial_variant->component_data[i][actor_data.data_index * sizeof(Component_T)]);

                //@TODO : don't reset memory every time
                auto* shrunken_data = new ComponentDataType[initial_variant->per_component_data_size[i] - old_component_data_size];
                initial_variant->per_component_data_size[i] -= old_component_data_size;

                // Copy old memory into the new one, excluding the removed component
                for (size_t e = 0, ei = 0; e < initial_variant->linked_actors.size(); ++e)
                {
                    if (e == actor_data.data_index)
                        continue;

                    old_component->component_move(&initial_variant->component_data[i][e * old_component_data_size], &shrunken_data[ei * old_component_data_size]);
                    old_component->component_destroy(&initial_variant->component_data[i][e * old_component_data_size]);

                    ++ei;
                }

                delete[] initial_variant->component_data[i];
                initial_variant->component_data[i] = shrunken_data;
            }
        }

        // Find actor that have been erased
        auto moved_back_actors = std::ranges::find(initial_variant->linked_actors, to_actor);

        // Move index back to erase the removed one
        std::for_each(moved_back_actors, initial_variant->linked_actors.end(), [this, &initial_variant](const ActorID& eid) {
            ActorMetaData& moveR = actor_meta_data[eid];
            --moveR.data_index;
        });

        // Remove the last index
        initial_variant->linked_actors.erase(moved_back_actors);
    }

    final_variant->linked_actors.emplace_back(to_actor);
    actor_data.data_index = final_variant->linked_actors.size() - 1;
    actor_data.variant    = final_variant;

    return component_memory;
}

template <class Component_T, typename... CtorArgs_T> void ECS::remove_component(const ActorID& from_actor)
{
    // The given component is not registered
    if (!is_component_type_registered<Component_T>())
        return;

    // The given actor does not exists
    if (!actor_meta_data.contains(from_actor))
        return;

    const ComponentTypeID compTypeId      = TComponent<Component_T>::get_type_id();
    ActorMetaData&        actor_data      = actor_meta_data[from_actor];
    ActorVariant*         initial_variant = actor_data.variant;

    // The component list is empty
    if (!initial_variant)
        return;

    // ensure the given entity have this component
    if (std::ranges::find(initial_variant->variant_specification, compTypeId) == initial_variant->variant_specification.end())
        return;

    // 1) Find an existing variant with given specs or create a new one
    std::vector<ComponentTypeID> final_variant_specs = initial_variant->variant_specification;
    final_variant_specs.erase(std::ranges::remove(final_variant_specs, compTypeId).begin(), final_variant_specs.end());
    std::ranges::sort(final_variant_specs);
    ActorVariant* final_variant = find_variant(final_variant_specs);

    for (std::size_t j = 0; j < final_variant_specs.size(); ++j)
    {
        const ComponentTypeID&  new_component_id        = final_variant_specs[j];
        const IComponent* const new_component           = component_registry[new_component_id];
        const std::size_t&      new_component_data_size = new_component->type_size();

        const std::size_t currentSize = final_variant->linked_actors.size() * new_component_data_size;
        new_component->resize_component_memory(currentSize + new_component_data_size, final_variant, j);

        std::vector<ComponentTypeID> old_component_spec = initial_variant->variant_specification;

        for (std::size_t i = 0; i < initial_variant->variant_specification.size(); ++i)
        {
            const ComponentTypeID old_component_id = initial_variant->variant_specification[i];

            if (old_component_id == new_component_id)
            {
                const std::size_t& oldCompDataSize = component_registry[old_component_id]->type_size();

                IComponent* moved_component = component_registry[old_component_id];
                moved_component->component_move(&initial_variant->component_data[i][actor_data.data_index * oldCompDataSize], &final_variant->component_data[j][currentSize]);
                moved_component->component_destroy(&initial_variant->component_data[i][actor_data.data_index * oldCompDataSize]);
                break;
            }
        }
    }

    for (std::size_t i = 0; i < initial_variant->variant_specification.size(); ++i)
    {
        const ComponentTypeID& oldCompTypeID = initial_variant->variant_specification[i];

        // if this is the component being removed, we should also destruct it
        if (oldCompTypeID == compTypeId)
            component_registry[compTypeId]->component_destroy(&initial_variant->component_data[i][actor_data.data_index * sizeof(Component_T)]);

        const IComponent* oldComp = component_registry[oldCompTypeID];

        const std::size_t oldCompDataSize = oldComp->type_size();

        initial_variant->per_component_data_size[i] -= oldCompDataSize;
        auto* newData = new ComponentDataType[initial_variant->per_component_data_size[i]];
        for (std::size_t e = 0, ei = 0; e < initial_variant->linked_actors.size(); ++e)
        {
            if (e == actor_data.data_index)
                continue;

            oldComp->component_move(&initial_variant->component_data[i][e * oldCompDataSize], &newData[ei * oldCompDataSize]);
            oldComp->component_destroy(&initial_variant->component_data[i][e * oldCompDataSize]);
            ++ei;
        }

        delete[] initial_variant->component_data[i];

        initial_variant->component_data[i] = newData;
    }

    // each entity in the old archetypes entityIds after this one now
    // has an index 1 less
    auto willBeRemoved = std::ranges::find(initial_variant->linked_actors, from_actor);

    std::for_each(willBeRemoved, initial_variant->linked_actors.end(), [this, &initial_variant](const ActorID& eid) {
        ActorMetaData& moveR = actor_meta_data[eid];
        --moveR.data_index;
    });

    initial_variant->linked_actors.erase(std::ranges::remove(initial_variant->linked_actors, from_actor).begin(), initial_variant->linked_actors.end());

    final_variant->linked_actors.push_back(from_actor);
    actor_data.data_index = final_variant->linked_actors.size() - 1;
    actor_data.variant    = final_variant;
}

} // namespace ecs