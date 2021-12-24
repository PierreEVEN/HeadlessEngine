#include "ecs_benchmark.h"
#include "entt_benchmark.h"
#include "raw_benchmark.h"
#include "ecs/actor.h"

#include "ecs/ecs.h"

#include <cpputils/logger.hpp>

#include "entt/entt.hpp"

int main()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    /*
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
    */

    LOG_VALIDATE("Starting performance tests with %d entities", BENCH_ENTITIES);
    LOG_VALIDATE("<<<<<< TESTING RAW EXAMPLES >>>>>>");
    raw_bench::create_entities();
    raw_bench::iterate_entities();
    raw_bench::destroy_entities();

    LOG_VALIDATE("<<<<<< TESTING ENTT >>>>>>");
    entt_bench::create_entities();
    entt_bench::iterate_entities();
    entt_bench::destroy_entities();

    LOG_VALIDATE("<<<<<< TESTING ECS >>>>>>");
    ecs_bench::create_entities();
    ecs_bench::iterate_entities();
    ecs_bench::destroy_entities();


    LOG_VALIDATE("finnished performance tests");
    exit(EXIT_SUCCESS);
}
