#include "ecs_benchmark.h"

#include "ecs/actor.h"
#include "ecs/ecs.h"
#include "profiler/profiler.h"

#include <cpputils/logger.hpp>

namespace ecs_bench
{
std::vector<ecs::Actor> actors;

void create_entities()
{
    Profiler prof;
    actors.resize(BENCH_ENTITIES);
    for (int i = 0; i < BENCH_ENTITIES; ++i)
        actors[i].add_component<FirstComponent>(10.f);
    LOG_INFO("CREATE : %lf ms", prof.get_ms());
}

void iterate_entities()
{
    Profiler prof1;
    for (const auto& variant : ecs::singleton().get_variants())
        for (size_t i = 0; i < variant->components.size(); ++i)
            if (variant->components[i].component_type->tick_runner) // Only if the component implement the tick method
                variant->components[i].component_type->tick_runner->execute(variant->components[i].component_data.data(), variant->linked_actors.size());
    LOG_INFO("RUN SLOW : %lf ms", prof1.get_ms());

    Profiler prof2;
    ecs::singleton().get_system_factory()->execute_tick();
    LOG_INFO("RUN FAST : %lf ms", prof2.get_ms());
}

void destroy_entities()
{
    Profiler prof;
    actors.clear();
    LOG_INFO("DESTROY : %lf ms", prof.get_ms());
}
} // namespace ecs_bench