#pragma once
#include "component_meta_class.h"

#include <cpputils/logger.hpp>
#include <unordered_map>

namespace ecs
{
class MyComponent
{
  public:
    void pre_render();
    void render(gfx::CommandBuffer* command_buffer);
    int  internal_val;
};

void ecs_test();

using EcsID           = uint32_t;
using ActorID         = EcsID;
using ComponentTypeID = EcsID;

template <typename BaseClass_T> class TypeIdGenerator
{
  public:
    template <class Type_T> static EcsID get()
    {
        static const EcsID class_id = current_id++;
        return class_id;
    }

  private:
    static EcsID current_id;
};

template <class BaseClass_T> EcsID TypeIdGenerator<BaseClass_T>::current_id = 0;

using ComponentDataType = uint8_t;

class IComponent
{
  public:
    virtual ~IComponent()                                                                          = default;
    virtual void   component_destroy(ComponentDataType* data) const                                = 0;
    virtual void   component_move(ComponentDataType* source, ComponentDataType* destination) const = 0;
    virtual void   component_ctor(ComponentDataType* data) const                                   = 0;
    virtual size_t type_size() const                                                               = 0;

    void resize_component_memory(size_t new_size, struct ActorVariant* variant, size_t component_type_index) const;
};

template <typename Component_T> class TComponent final : public IComponent
{
  public:
    void component_destroy(ComponentDataType* component_ptr) const override
    {
        Component_T* fixed_component_ptr = std::launder(reinterpret_cast<Component_T*>(component_ptr));
        fixed_component_ptr->~Component_T();
    }

    void component_move(ComponentDataType* source, ComponentDataType* destination) const override
    {
        new (&destination[0]) Component_T(std::move(*reinterpret_cast<Component_T*>(source)));
    }

    void component_ctor(ComponentDataType* component_ptr) const override
    {
        (void)component_ptr;
        //@TODO wtf
        // new (&component_ptr[0]) Component_T();
    }

    static ComponentTypeID get_type_id()
    {
        return TypeIdGenerator<IComponent>::get<Component_T>();
    }
    size_t type_size() const override
    {
        return sizeof(Component_T);
    }
};
struct ActorVariant
{
    std::vector<ComponentTypeID> variant_specification;

    std::vector<ComponentDataType*> component_data;
    std::vector<ActorID>            linked_actors;
    std::vector<std::size_t>        per_component_data_size;
};

struct ActorMetaData
{
    ActorVariant* variant;
    std::size_t   data_index;
};

class ECS final
{
  private:
    ECS() = default;

  public:
    static ECS& get();
    ~ECS()
    {
        for (auto& registry : component_registry)
            delete registry.second;
    }

    template <class Component_T> bool is_component_type_registered()
    {
        return component_registry.contains(TComponent<Component_T>::get_type_id());
    }

    template <class Component_T> void register_component_type()
    {
        if (is_component_type_registered<Component_T>())
            return;

        component_registry[TComponent<Component_T>::get_type_id()] = new TComponent<Component_T>();

        LOG_INFO("registered component type %s", typeid(Component_T).name());
    }

    template <class Component_T, typename... CtorArgs_T> Component_T* add_component(const ActorID& to_actor, CtorArgs_T&&... args);

    template <class Component_T, typename... CtorArgs_T> Component_T* remove_component(const ActorID& from_actor);

    ActorVariant* find_variant(const std::vector<ComponentTypeID>& variant_spec)
    {
        LOG_INFO("searching variant");
        for (ActorVariant* archetype : variant_registry)
            if (archetype->variant_specification == variant_spec)
                return archetype;

        LOG_INFO("failed to find existing variant. Creating a new one");
        ActorVariant* new_variant          = new ActorVariant();
        new_variant->variant_specification = variant_spec;
        variant_registry.emplace_back(new_variant);

        for (size_t i = 0; i < variant_spec.size(); ++i)
        {
            new_variant->component_data.emplace_back(new ComponentDataType[0]);
            new_variant->per_component_data_size.emplace_back(0);
        }

        return new_variant;
    }

    void add_empty_actor(const ActorID actor_instance_id)
    {
        ActorMetaData actor_data;
        actor_data.variant                 = nullptr;
        actor_data.data_index              = 0;
        actor_meta_data[actor_instance_id] = actor_data;
        LOG_INFO("Created empty actor");
    }

    ActorID make_new_actor_id()
    {
        return last_actor_id++;
    }

  private:
    void remove_actor(const ActorID actor_instance_id)
    {
        (void)actor_instance_id; //@TODO
    }

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
    const ComponentTypeID component_type_id = TComponent<Component_T>::get_type_id();
    const size_t          type_size         = component_registry[component_type_id]->type_size();
    ActorMetaData&        actor_data        = actor_meta_data[to_actor];
    ActorVariant*         initial_variant   = actor_data.variant;
    ActorVariant*         final_variant     = nullptr;

    // Actor doesn't have components : we initialize it
    if (!initial_variant)
    {
        std::vector<ComponentTypeID> new_variant_spec = {component_type_id};
        const IComponent*            component_type   = component_registry[component_type_id];

        final_variant = find_variant(new_variant_spec); //@TODO implement method

        const std::size_t actual_data_size = final_variant->linked_actors.size() * type_size;

        component_type->resize_component_memory(actual_data_size + type_size, final_variant, 0);

        component_memory = new (&final_variant->component_data[0][actual_data_size]) Component_T(std::forward<CtorArgs_T>(args)...);

        LOG_INFO("constructed new variant and initialized first component");
    }
    else // We need to find a new variant for the given actor
    {
        // Check if the current variant already contains the specified component
        if (std::ranges::find(initial_variant->variant_specification, component_type_id) != initial_variant->variant_specification.end())
        {
            // Cannot support multiple components of the same type within the same actor
            LOG_ERROR("actor already contains a component of type %s", typeid(Component_T).name());
            return nullptr;
        }

        // Add component to the existing spec
        std::vector<ComponentTypeID> updated_spec = initial_variant->variant_specification;
        updated_spec.emplace_back(component_type_id);
        std::ranges::sort(updated_spec); // don't forget too always sort variant to avoid duplications

        final_variant = find_variant(updated_spec); // find or create variant with given specs

        // Copy initial variant data into the new one
        for (std::size_t j = 0; j < updated_spec.size(); ++j)
        {
            const ComponentTypeID& component_id            = updated_spec[j];
            const IComponent*      new_component_type      = component_registry[component_id];
            const std::size_t      new_component_type_size = new_component_type->type_size();
            const std::size_t      initial_size            = final_variant->linked_actors.size() * new_component_type_size;

            new_component_type->resize_component_memory(initial_size + new_component_type_size, final_variant, j);

            // If it is data from the old variant
            for (std::size_t i = 0; i < initial_variant->variant_specification.size(); ++i)
            {
                const ComponentTypeID& oldCompId = initial_variant->variant_specification[i];
                if (oldCompId == component_id)
                {
                    const IComponent* oldComp = component_registry[oldCompId];

                    const std::size_t& oldCompDataSize = oldComp->type_size();

                    oldComp->component_move(&initial_variant->component_data[i][actor_data.data_index * oldCompDataSize], &final_variant->component_data[j][initial_size]);
                    oldComp->component_destroy(&initial_variant->component_data[i][actor_data.data_index * oldCompDataSize]);

                    goto cnt;
                }
            }

            component_memory = new (&final_variant->component_data[j][initial_size]) Component_T(std::forward<CtorArgs_T>(args)...);

        cnt:;
        }

        if (!initial_variant->linked_actors.empty())
        {
            for (std::size_t i = 0; i < initial_variant->variant_specification.size(); ++i)
            {
                const ComponentTypeID& oldCompTypeID = initial_variant->variant_specification[i];

                if (oldCompTypeID == component_type_id)
                {
                    const IComponent* removeWrapper = component_registry[component_type_id];
                    removeWrapper->component_destroy(&initial_variant->component_data[i][actor_data.data_index * sizeof(Component_T)]);
                }

                const IComponent* oldComp = component_registry[oldCompTypeID];

                const std::size_t& oldCompDataSize = oldComp->type_size();

                auto* newData = new ComponentDataType[initial_variant->per_component_data_size[i] - oldCompDataSize];
                initial_variant->per_component_data_size[i] -= oldCompDataSize;
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
        }

        // Erase obsolete data by moving back
        auto moved_back_actors = std::ranges::find(initial_variant->linked_actors, to_actor);

        std::for_each(moved_back_actors, initial_variant->linked_actors.end(),
                      [this, &initial_variant](const ActorID& eid)
                      {
                          ActorMetaData& moveR = actor_meta_data[eid];
                          --moveR.data_index;
                      });

        initial_variant->linked_actors.erase(moved_back_actors);
    }

    final_variant->linked_actors.emplace_back(to_actor);
    actor_data.data_index = final_variant->linked_actors.size() - 1;
    actor_data.variant    = final_variant;

    LOG_INFO("added component %s to entity at index.", typeid(Component_T).name(), actor_data.data_index, actor_data.variant);

    return component_memory;
}

template <class Component_T, typename... CtorArgs_T> Component_T* ECS::remove_component(const ActorID& from_actor)
{
    if (!is_component_type_registered<Component_T>())
        return;
}
} // namespace ecs