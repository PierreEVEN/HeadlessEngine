#include "entt_benchmark.h"

#include "entt/entt.hpp"
#include "profiler/profiler.h"

namespace entt_bench
{

std::unique_ptr<entt::registry> registry;
std::vector<entt::entity>       entities;

void create_entities()
{
    Profiler prof;
    registry = std::make_unique<entt::registry>();

    entities.resize(BENCH_ENTITIES);

    for (size_t i = 0; i < BENCH_ENTITIES; ++i)
    {
        entities[i] = registry->create();
        registry->emplace<FirstComponent>(entities[i], 20);
    }
    LOG_INFO("CREATE : %lf ms", prof.get_ms());
}

void iterate_entities()
{
    Profiler prof1;
    auto     view_slow = registry->view<FirstComponent>();
    for (auto [entity, component] : view_slow.each())
    {
        component.value++;
    }
    LOG_INFO("RUN A : %lf ms", prof1.get_ms());

    Profiler prof2;
    auto view_fast = registry->view<FirstComponent>();
    view_fast.each(
        [](FirstComponent& component)
        {
        component.value++;
        });
    LOG_INFO("RUN B : %lf ms", prof2.get_ms());

    /*
    Profiler prof3;
        for (const entt::entity entity : registry->view<FirstComponent>() )
        {
            registry->get<FirstComponent>(entity)

        });
    LOG_INFO("RUN B : %lf ms", prof2.get_ms());
    */
}

void destroy_entities()
{
    Profiler prof;
    for (size_t i = 0; i < BENCH_ENTITIES; ++i)
        registry->destroy(entities[i]);
    registry = nullptr;
    LOG_INFO("DESTROY : %lf ms", prof.get_ms());
}
} // namespace entt_bench