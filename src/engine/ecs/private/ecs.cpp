#include "ecs/ecs.h"

#include "ecs/actor.h"

namespace ecs
{
std::unique_ptr<ECS> ecs_singleton;

ECS& ECS::get()
{
    if (!ecs_singleton)
        ecs_singleton = std::unique_ptr<ECS>(new ECS());
    return *ecs_singleton.get();
}

ECS::~ECS()
{
    for (const ActorVariant* variant : variant_registry)
    {
        for (size_t i = 0; i < variant->variant_specification.size(); ++i)
        {
            const IComponent* const component           = component_registry[variant->variant_specification[i]];
            const size_t&           component_item_size = component->type_size();
            for (size_t e = 0; e < variant->linked_actors.size(); ++e)
            {
                component->component_destroy(&variant->component_data[i][e * component_item_size]);
            }
            delete[] variant->component_data[i];
        }
        delete variant;
    }
    for (auto& comp : component_registry)
        delete comp.second;
}

ActorVariant* ECS::find_variant(const std::vector<ComponentTypeID>& variant_spec)
{
    for (ActorVariant* archetype : variant_registry)
        if (archetype->variant_specification == variant_spec)
            return archetype;

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

void ECS::add_empty_actor(const ActorID actor_instance_id)
{
    ActorMetaData actor_data;
    actor_data.variant                 = nullptr;
    actor_data.data_index              = 0;
    actor_meta_data[actor_instance_id] = actor_data;
}

void ECS::remove_actor(const ActorID& removed_actor)
{
    if (!actor_meta_data.contains(removed_actor))
        return; // it doesn't exist

    ActorMetaData& record = actor_meta_data[removed_actor];

    ActorVariant* oldArchetype = record.variant;

    if (!oldArchetype)
    {
        actor_meta_data.erase(removed_actor);
        return; // we wouldn't know where to delete
    }

    for (std::size_t i = 0; i < oldArchetype->variant_specification.size(); ++i)
    {
        const ComponentTypeID& oldCompId = oldArchetype->variant_specification[i];

        const IComponent* const comp = component_registry[oldCompId];

        const std::size_t& compSize = comp->type_size();

        comp->component_destroy(&oldArchetype->component_data[i][record.data_index * compSize]);
    }

    for (std::size_t i = 0; i < oldArchetype->variant_specification.size(); ++i)
    {
        const ComponentTypeID& oldCompID = oldArchetype->variant_specification[i];
        const IComponent* const oldComp = component_registry[oldCompID];
        const std::size_t& oldCompDataSize = oldComp->type_size();

        oldComp->resize_component_memory((oldArchetype->linked_actors.size()) * oldCompDataSize, oldArchetype, i);
    }

    actor_meta_data.erase(removed_actor);

    const auto removed_iterator = std::ranges::find(oldArchetype->linked_actors, removed_actor);

    std::for_each(removed_iterator, oldArchetype->linked_actors.end(),
                  [this, &oldArchetype, &removed_actor](const ActorID& eid)
                  {
                      if (eid == removed_actor)
                          return; // no need to adjust our removing one
                      ActorMetaData& moveR = actor_meta_data[eid];
                      moveR.data_index -= 1;
                  });

    oldArchetype->linked_actors.erase(removed_iterator);
}

ActorID ECS::make_new_actor_id()
{
    return last_actor_id++;
}

void ECS::tick()
{
    auto now = std::chrono::steady_clock::now();
    // Execute tick, pre-render, render method for every components
    for (const auto& variant : ECS::get().get_variants())
    {
        for (size_t i = 0; i < variant->variant_specification.size(); ++i)
        {
            const IComponent* component_type = ECS::get().get_component_type(variant->variant_specification[i]);
            if (component_type->tick_runner) // Only if the component implement the tick method
                component_type->tick_runner->execute(variant->component_data[i], variant->linked_actors.size());
        }
    }
    LOG_DEBUG("ECS : iterate over components using a callback (Should be close to 2-b): \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

    now = std::chrono::steady_clock::now();
    system_factory.execute_tick();
    LOG_DEBUG("ECS : iterate over components using a System (Should be close to 2-c) : \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());
}

void ECS::pre_render()
{
    // Execute tick, pre-render, render method for every components
    for (const auto& variant : ECS::get().get_variants())
    {
        for (size_t i = 0; i < variant->variant_specification.size(); ++i)
        {
            const IComponent* component_type = ECS::get().get_component_type(variant->variant_specification[i]);
            if (component_type->pre_render_runner) // Only if the component implement the tick method
                component_type->pre_render_runner->execute(variant->component_data[i], variant->linked_actors.size());
        }
    }
}

void ECS::render(gfx::CommandBuffer* command_buffer)
{
    // Execute tick, pre-render, render method for every components
    for (const auto& variant : ECS::get().get_variants())
    {
        for (size_t i = 0; i < variant->variant_specification.size(); ++i)
        {
            const IComponent* component_type = ECS::get().get_component_type(variant->variant_specification[i]);
            if (component_type->render_runner) // Only if the component implement the tick method
                component_type->render_runner->execute(variant->component_data[i], variant->linked_actors.size(), command_buffer);
        }
    }
}

struct MyComp
{
    MyComp(float in_val) : val(in_val)
    {
    }

    void tick()
    {
        val++;
    }

    static void add_systems(SystemFactory* factory)
    {
        factory->tick<MyComp>(
            [](TSystemIterable<MyComp> iterator)
            {
                for (auto [comp1] : iterator)
                {
                    comp1.val++;
                }
            });
    }

  private:
    float val;
};

struct MyComp2
{
    MyComp2(float in_val) : val(in_val)
    {
    }

    void tick()
    {
        val--;
    }

  private:
    float val;
};

void ecs_test()
{
    ECS::get().register_component_type<MyComp>();
    ECS::get().register_component_type<MyComp2>();

    std::vector<Actor*> actors;

    auto now = std::chrono::steady_clock::now();
    for (int i = 0; i < TEST_N; ++i)
    {
        Actor* new_actor = new Actor;
        new_actor->add_component<MyComp>(10.f);
        //new_actor->add_component<MyComp2>(10.f);
        actors.emplace_back(new_actor);
    }
    LOG_DEBUG("Created %d entities in %d us", TEST_N, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

    
    LOG_WARNING("##### RUNNING ECS PERF TEST (%d entities) #####", TEST_N);
    // example game loop
    do
    {
        gfx::CommandBuffer* command_buffer = {}; // @TODO
        ECS::get().tick();
        ECS::get().pre_render();
        ECS::get().render(command_buffer);
    } while (false);

    now = std::chrono::steady_clock::now();
    for (auto& actor : actors)
    {
        delete actor;
    }
    LOG_DEBUG("Destroy %d entities in %d us (disabled)", TEST_N, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());
}
} // namespace ecs
