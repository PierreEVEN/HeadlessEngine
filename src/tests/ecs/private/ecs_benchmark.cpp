#include "ecs_benchmark.h"

#include "ecs/ecs.h"
#include <cpputils/logger.hpp>



void TestComponent::test_func()
{
    local_var++;
}

void TestComponentParent::test_func()
{
    local_var++;
}

void DerivComponent::test_func()
{
    local_var--;
}

void perf_test()
{
    LOG_WARNING("##### RUNNING BENCHMARK TESTS (%d entities) #####", TEST_N);
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);
    {
        std::vector<DerivComponent> deriv_comp;
        deriv_comp.resize(TEST_N);
        for (int i = 0; i < TEST_N; ++i)
            new (&deriv_comp[i]) DerivComponent();

        std::vector<TestComponentParent*> rand_comp;
        rand_comp.resize(TEST_N);
        for (auto& comp : rand_comp)
            comp = new DerivComponent();

        auto now = std::chrono::steady_clock::now();
        for (auto& comp : rand_comp)
            comp->test_func();
        LOG_WARNING("1) Using a virtual method");

        LOG_DEBUG("a. Virtual method call on randomly allocated memory : \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : deriv_comp)
            comp.test_func();
        LOG_DEBUG("b. Virtual method call on continuous allocated memory : \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : deriv_comp)
            comp.local_var++;
        LOG_DEBUG("c. iterate over components with continuous allocated memory : \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : rand_comp)
            comp->local_var++;
        LOG_DEBUG("d. iterate over components with randomly allocated memory : \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        for (int i = 0; i < TEST_N; ++i)
            delete rand_comp[i];
    }
    {
        std::vector<TestComponent> deriv_comp;
        deriv_comp.resize(TEST_N);
        for (int i = 0; i < TEST_N; ++i)
            new (&deriv_comp[i]) TestComponent();

        std::vector<TestComponent*> rand_comp;
        rand_comp.resize(TEST_N);
        for (auto& comp : rand_comp)
            comp = new TestComponent();

        auto now = std::chrono::steady_clock::now();
        for (const auto& comp : rand_comp)
            comp->test_func();
        LOG_WARNING("2) Without using a virtual method");

        LOG_DEBUG("a. method call on randomly allocated memory : \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : deriv_comp)
            comp.test_func();

        LOG_DEBUG("b. method call on continuous allocated memory (Best theoretical perfs using an ECS on a callback) : \n\t\t=> %d us",
                  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (auto& comp : deriv_comp)
            comp.local_var++;
        LOG_DEBUG("c. iterate over components with continuous allocated memory (Best theoretical perfs using the full ECS) : \n\t\t=> %d us",
                  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        now = std::chrono::steady_clock::now();
        for (const auto& comp : rand_comp)
            comp->local_var++;
        LOG_DEBUG("d. iterate over components with randomly allocated memory : \n\t\t=> %d us", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count());

        for (int i = 0; i < TEST_N; ++i)
            delete rand_comp[i];
    }
}
