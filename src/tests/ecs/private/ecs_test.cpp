#include "ecs_benchmark.h"
#include "ecs/actor.h"

#include "ecs/ecs.h"

#include <cpputils/logger.hpp>

#include "entt/entt.hpp"

struct MyComp
{
    MyComp(float in_val) : val(in_val)
    {
    }

    void tick()
    {
        val++;
    }
    
    static void add_systems(ecs::SystemFactory* factory)
    {
        factory->tick<MyComp>(
            [](ecs::TSystemIterable<MyComp> iterator)
            {
                for (auto [entity, comp1] : iterator)
                {
                    comp1.val = static_cast<float>(*entity);
                }
            });
    }

    float val;
  private:
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

int main()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    ecs::ECS::get().register_component_type<MyComp>();
    ecs::ECS::get().register_component_type<MyComp2>();

    ecs::Actor act1;
    act1.add_component<MyComp>(10.f);

    ecs::ECS::get().tick();

    LOG_INFO("test 1");
    ecs::ECS::get().render(nullptr);

    LOG_INFO("test 2");
    ecs::Actor act2;
    act2.add_component<MyComp>(120.f);
    ecs::ECS::get().render(nullptr);

    
    LOG_INFO("test 3");
    act2.add_component<MyComp2>(140.f);
    act1.add_component<MyComp2>(20.f);

    ecs::Actor act3;
    act3.add_component<MyComp>(220.f);
    act3.add_component<MyComp2>(240.f);

    ecs::ECS::get().render(nullptr);
    ecs::ECS::get().tick();

    act1.remove_component<MyComp>();

    ecs::ECS::get().render(nullptr);



    act1.~Actor();
    act2.~Actor();
    act3.~Actor();



    LOG_WARNING("######### ------ PERFORMANCE TESTS ------ #########");
    perf_test();

    LOG_WARNING("###### ENTITY PERF TESTS ######");
    entt::registry registry;

    auto now = std::chrono::steady_clock::now();
    for (size_t i = 0; i < TEST_N; ++i)
    {
        const auto entity = registry.create();
        registry.emplace<MyComp>(entity, 20);
    }
    LOG_DEBUG("Created %d entities in %d us", TEST_N, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

    auto view = registry.view<MyComp>();
    now = std::chrono::steady_clock::now();
    view.each(
        [](MyComp& my_comp)
        {
            my_comp.val++;
        });
    LOG_DEBUG("Run entt loop : (%d entities) %d us", TEST_N, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

    now                     = std::chrono::steady_clock::now();
    const auto velocityView = registry.view<MyComp>();
    for (const entt::entity entity : velocityView)
    {
        velocityView.get<MyComp>(entity).val++;
    }
    LOG_DEBUG("Run entt loop V2 : (%d entities) %d us", TEST_N, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

    registry.~basic_registry();
    // registry.destroy();

    LOG_WARNING("##### RUNNING ECS PERF TEST (%d entities) #####", TEST_N);
    
    std::vector<ecs::Actor*> actors;
    actors.resize(TEST_N);

    now = std::chrono::steady_clock::now();
    for (int i = 0; i < TEST_N; ++i)
    {
        const auto new_actor = new ecs::Actor();
        new_actor->add_component<MyComp>(10.f);
        actors[i] = new_actor;
    }
    LOG_DEBUG("Created %d entities in %d us", TEST_N, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

    // example game loop
    do
    {
        gfx::CommandBuffer* command_buffer = {}; // @TODO
        ecs::ECS::get().tick();
        ecs::ECS::get().pre_render();
        ecs::ECS::get().render(command_buffer);
    } while (false);

    now = std::chrono::steady_clock::now();
    for (const auto& actor : actors)
        delete actor;
    LOG_DEBUG("Destroy %d entities in %d us", TEST_N, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());


    exit(EXIT_SUCCESS);
}
