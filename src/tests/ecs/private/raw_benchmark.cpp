#include "raw_benchmark.h"

#include "profiler/profiler.h"

namespace raw_bench
{
std::vector<FirstComponent> contiguous_components;

void create_entities()
{
    Profiler prof;
    contiguous_components.resize(BENCH_ENTITIES);
    for (int i = 0; i < BENCH_ENTITIES; ++i)
        new (&contiguous_components[i]) FirstComponent(10);    
    LOG_INFO("CREATE : %lf ms", prof.get_ms());

}

void iterate_entities()
{
    Profiler prof1;
    for (auto& comp : contiguous_components)
        comp.tick();
    LOG_INFO("RUN SLOW : %lf ms", prof1.get_ms());

    Profiler prof2;
    for (auto& comp : contiguous_components)
        comp.value++;
    LOG_INFO("RUN FAST : %lf ms", prof2.get_ms());
}

void destroy_entities()
{
    Profiler prof;
    contiguous_components.clear();
    LOG_INFO("DESTROY : %lf ms", prof.get_ms());
}
} // namespace raw_bench
