#include "application/application.h"
#include "application/window.h"

#include <gfx/gfx.h>

#include <thread>

#include <cpputils/logger.hpp>

int main()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    application::create();
    application::window::create_window(application::window::WindowConfig{.name = application::get_full_name(), .window_style = application::window::EWindowStyle::WINDOWED});
    application::window::create_window(application::window::WindowConfig{.name = application::get_engine_full_name()});
    
    while (application::window::Window::get_window_count() > 0)
    {
        for (uint32_t i = 0; i < application::window::Window::get_window_count(); ++i)
            application::window::Window::get_window(i)->update();
    }

    application::destroy();

    exit(EXIT_SUCCESS);

    /*
    gfx::init();
    std::unique_ptr<gfx::Buffer> gpu_buffer = std::make_unique<gfx::Buffer>(64, gfx::EBufferUsage::GPU_MEMORY);    
    gpu_buffer = nullptr;
    gfx::destroy();
    */
}
