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
