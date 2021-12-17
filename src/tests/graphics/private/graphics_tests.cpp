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
        
    gfx::init();

    std::unique_ptr<gfx::Buffer> gpu_buffer      = std::make_unique<gfx::Buffer>("gpu_buffer",  64, gfx::EBufferUsage::GPU_MEMORY);
    std::unique_ptr<gfx::Buffer> indirect_buffer = std::make_unique<gfx::Buffer>("indirect_buffer", 64, gfx::EBufferUsage::INDIRECT_DRAW_ARGUMENT);
    int32_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    gpu_buffer->set_data(data, sizeof(int32_t) * 16);
    indirect_buffer->set_data(data, sizeof(int32_t) * 16);

    while (application::window::Window::get_window_count() > 0)
    {
        for (uint32_t i = 0; i < application::window::Window::get_window_count(); ++i)
            application::window::Window::get_window(i)->update();
    }

    gpu_buffer = nullptr;
    indirect_buffer = nullptr;

    gfx::destroy();
    application::destroy();

    exit(EXIT_SUCCESS);
}
