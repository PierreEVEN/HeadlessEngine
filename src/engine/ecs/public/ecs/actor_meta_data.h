#pragma once
#include "ecs/component.h"

#include <cpputils/logger.hpp>
#include <vector>

namespace ecs
{
class ActorMetaData final
{
  public:
    class ActorVariant* variant    = nullptr;
    uint32_t            data_index = UINT32_MAX;
    ActorID             actor_id   = {};
};

class ActorVariant final
{
  public:
    ActorVariant(ECS* context, const std::vector<ComponentTypeID>& in_specification);

    void emplace_actor_back(ActorMetaData* actor);
    void remove_actor(ActorMetaData* actor, bool only_move);

    [[nodiscard]] ComponentDataType* get_component_memory(ComponentTypeID type_id, uint32_t element_index)
    {
        for (auto& component : components)
            if (component.type_id == type_id)
            {
                return &component.component_data_buffer[element_index * component.type_size];
            }

        LOG_FATAL("failed to find component type with the given type_id");
    }

    [[nodiscard]] const std::vector<ComponentTypeID>& get_specification() const
    {
        return specification;
    }

    static void move_actor_to_variant(ActorMetaData* actor, ActorVariant* from, ActorVariant* to);

    void duplicate_actor(const ActorMetaData* existing_actor, ActorMetaData* new_actor);

  private:
    template <typename... Component_T> friend class TSystemIterable;
    friend ECS;

    struct ComponentData
    {
        std::size_t                    type_size;
        std::vector<ComponentDataType> component_data_buffer;
        ComponentTypeID                type_id;
        IComponentHelper*              component_type;
    };

    const std::vector<ComponentTypeID> specification;
    std::vector<ComponentData>         components;
    std::vector<ActorID>               linked_actors;
    ECS*                               owning_ecs;

    void update_components_buffer_size();
};
} // namespace ecs