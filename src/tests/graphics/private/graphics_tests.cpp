#include "application/application.h"
#include "application/window.h"
#include "gfx/surface.h"

#include <gfx/gfx.h>

#include <thread>

#include <cpputils/logger.hpp>

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

int main()
{
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
            new (&deriv_comp[i]) TestComponent();

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
    exit(EXIT_SUCCESS);

    /**
     * 1° initialize the application and the gfx backend
     */
    application::create();
    gfx::init();

    /**
     * 2° create some windows with surfaces (surfaces are layers that allow rendering images from the gfx backend onto application window)
     */
    auto* window_1  = application::window::create_window(application::window::WindowConfig{.name = application::get_full_name(), .window_style = application::window::EWindowStyle::WINDOWED});
    auto  surface_1 = std::unique_ptr<gfx::Surface>(gfx::Surface::create_surface(window_1));

    auto* window_2  = application::window::create_window(application::window::WindowConfig{.name = application::get_engine_full_name()});
    auto  surface_2 = std::unique_ptr<gfx::Surface>(gfx::Surface::create_surface(window_2));

    /**
     * 3° Load some data on the GPU
     */
    int32_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    std::unique_ptr<gfx::Buffer> gpu_buffer = std::make_unique<gfx::Buffer>("gpu_buffer", 64, gfx::EBufferUsage::GPU_MEMORY);
    gpu_buffer->set_data(data, sizeof(int32_t) * 16);

    std::unique_ptr<gfx::Buffer> indirect_buffer = std::make_unique<gfx::Buffer>("indirect_buffer", 64, gfx::EBufferUsage::INDIRECT_DRAW_ARGUMENT);
    indirect_buffer->set_data(data, sizeof(int32_t) * 16);

    /**
     * 4° Application loop
     */
    while (application::window::Window::get_window_count() > 0)
    {
        for (uint32_t i = 0; i < application::window::Window::get_window_count(); ++i)
        {
            gfx::CommandBuffer* cmd_buffer = nullptr;

            surface_1->submit_command_buffer(cmd_buffer);
            application::window::Window::get_window(i)->update();
        }
    }

    /**
     * 5° clean GPU data : //@TODO automatically free allocated resources
     */
    gpu_buffer      = nullptr;
    indirect_buffer = nullptr;
    surface_1       = nullptr;
    surface_2       = nullptr;

    // Destroy graphic backend and close application
    gfx::destroy();
    application::destroy();

    exit(EXIT_SUCCESS);
}
