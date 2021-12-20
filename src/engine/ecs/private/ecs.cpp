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

        ComponentDataType* newData = new unsigned char[oldArchetype->per_component_data_size[i] - oldCompDataSize];
        oldArchetype->per_component_data_size[i] -= oldCompDataSize;
        for (std::size_t e = 0, ei = 0; e < oldArchetype->linked_actors.size(); ++e)
        {
            if (e == record.data_index)
                continue;

            oldComp->component_move(&oldArchetype->component_data[i][e * oldCompDataSize], &newData[ei * oldCompDataSize]);

            oldComp->component_destroy(&oldArchetype->component_data[i][e * oldCompDataSize]);

            ++ei;
        }

        delete[] oldArchetype->component_data[i];

        oldArchetype->component_data[i] = newData;
    }

    actor_meta_data.erase(removed_actor);

    const auto removed_iterator = std::ranges::find(oldArchetype->linked_actors, removed_actor);

    std::for_each(removed_iterator, oldArchetype->linked_actors.end(), [this, &oldArchetype, &removed_actor](const ActorID& eid)
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


struct TestComponent
{
    void test_func()
    {
        local_var++;
    }

    int local_var;
};

struct TestComponentParent
{
    virtual void test_func()
    {
        local_var++;
    }

    int local_var;
};

struct DerivComponent : public TestComponentParent
{
    void test_func() override
    {
        local_var--;
    }
};

#define TEST_N 10000000

void perf_test()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);
    {
        std::vector<DerivComponent> deriv_comp;
        deriv_comp.resize(TEST_N);
        for (int i = 0; i < TEST_N; ++i)
            new(&deriv_comp[i]) DerivComponent();

        std::vector<TestComponentParent*> rand_comp;
        rand_comp.resize(TEST_N);
        for (auto& comp : rand_comp)
            comp = new DerivComponent();

        auto now = std::chrono::steady_clock::now();
        for (auto& comp : rand_comp)
            comp->test_func();
        LOG_WARNING(" AVEC APPEL DE METHODE VIRTUELLE");

        LOG_DEBUG("A) Tick sauce unreal : \n%d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : deriv_comp)
            comp.test_func();
        LOG_DEBUG("B) Tick a la unreal mais memoire contigue : \n%d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : deriv_comp)
            comp.local_var++;
        LOG_DEBUG("D) foreach sans appel de fonction et memoire contigue (ECS stonks) : \n%d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : rand_comp)
            comp->local_var++;
        LOG_DEBUG("E) foreach sans appel de fonction mais memoire pas contigue : \n%d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        for (int i = 0; i < TEST_N; ++i)
            delete rand_comp[i];
    }
    {
        std::vector<TestComponent> deriv_comp;
        deriv_comp.resize(TEST_N);
        for (int i = 0; i < TEST_N; ++i)
            new(&deriv_comp[i]) TestComponent();

        std::vector<TestComponent*> rand_comp;
        rand_comp.resize(TEST_N);
        for (auto& comp : rand_comp)
            comp = new TestComponent();

        auto now = std::chrono::steady_clock::now();
        for (auto& comp : rand_comp)
            comp->test_func();
        LOG_WARNING(" SANS APPEL DE METHODE VIRTUELLE");

        LOG_DEBUG("A) Tick sauce unreal : \n%d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : deriv_comp)
            comp.test_func();
        LOG_DEBUG("B) Tick a la unreal mais memoire contigue : \n%d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : deriv_comp)
            comp.local_var++;
        LOG_DEBUG("D) foreach sans appel de fonction et memoire contigue (ECS stonks) : \n%d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : rand_comp)
            comp->local_var++;
        LOG_DEBUG("E) foreach sans appel de fonction mais memoire pas contigue : \n%d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        for (int i = 0; i < TEST_N; ++i)
            delete rand_comp[i];
    }
}


struct MyComp
{
    MyComp(float val)
    {
        (void)val;
    }

    void tick()
    {
        val++;
    }

    int val;
};

struct MyComp2 : public MyComp
{
    MyComp2(float val)
        : MyComp(val)
    {
    }

    virtual void tick()
    {
        val--;
    }
};

void ecs_test()
{
    ECS::get().register_component_type<MyComp>();
    ECS::get().register_component_type<MyComp2>();

    std::vector<Actor*> actors;

    auto now = std::chrono::steady_clock::now();

    for (int i = 0; i < 10000000; ++i)
    {
        Actor* new_actor = new Actor;
        new_actor->add_component<MyComp>(10.f);
        actors.emplace_back(new_actor);
    }
    LOG_DEBUG("Create duration : %d", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

    perf_test();

    LOG_WARNING("ECS perfs");
    now = std::chrono::steady_clock::now();
    // example game loop
    do
    {
        gfx::CommandBuffer* command_buffer = {}; // @TODO

        // Execute tick, pre-render, render method for every components
        for (const auto& variant : ECS::get().get_variants())
        {
            for (size_t i = 0; i < variant->variant_specification.size(); ++i)
            {
                IComponent* component_type = ECS::get().get_component_type(variant->variant_specification[i]);

                if (component_type->tick_runner) // Only if the component implement the tick method
                    component_type->tick_runner->execute(variant->component_data[i], variant->linked_actors.size());

                if (component_type->pre_render_runner) // Only if the component implement the pre-render method
                    component_type->pre_render_runner->execute(variant->component_data[i], variant->linked_actors.size());

                if (component_type->render_runner) // Only if the component implement the render method
                    component_type->render_runner->execute(variant->component_data[i], variant->linked_actors.size(), command_buffer);
            }
        }

        LOG_DEBUG("total duration : %d", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        //@TODO add custom runners for maximal performances
    } while (false);

    now = std::chrono::steady_clock::now();
    for (auto& actor : actors)
    {
        //delete actor;
    }
    LOG_DEBUG("Destroy duration : %d", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());
}
} // namespace ecs
