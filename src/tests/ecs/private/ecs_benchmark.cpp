#include "ecs_benchmark.h"

#include "ecs/actor.h"
#include "ecs/ecs.h"
#include "profiler/profiler.h"

#include <cpputils/logger.hpp>

namespace ecs_bench
{

std::unique_ptr<ecs::ECS> instance;

std::vector<std::shared_ptr<ecs::Actor>> actors;

void create_entities()
{
    instance = std::make_unique<ecs::ECS>();
    const Profiler prof;
    actors.reserve(BENCH_ENTITIES);
    for (int i = 0; i < BENCH_ENTITIES; ++i)
    {
        actors[i] = instance->new_actor();
        actors[i]->add_component<FirstComponent>(10.f);
    }
    LOG_INFO("CREATE : %lf ms", prof.get_ms());
}

void iterate_entities()
{
    const Profiler prof1;
    instance->tick();
    LOG_INFO("RUN : %lf ms", prof1.get_ms());
}

void destroy_entities()
{
    const Profiler prof;
    actors.clear();
    instance = nullptr;
    LOG_INFO("DESTROY : %lf ms", prof.get_ms());
}
} // namespace ecs_bench