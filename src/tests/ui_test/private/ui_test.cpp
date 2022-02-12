

#include "application/application.h"
#include "application/window.h"
#include "ui.h"

#include "gfx/gfx.h"

#include <cpputils/logger.hpp>

#include <cpputils/eventmanager.hpp>

int main()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    /**
     * 1° initialize the application and the gfx backend
     */
    application::create();
    gfx::init();
    application::window::Window* window  = create_window(application::window::WindowConfig{.name = application::get_full_name(), .window_style = application::window::EWindowStyle::WINDOWED});
    gfx::Surface*                surface = gfx::Surface::create_surface("default_surface", window);

    std::unique_ptr<ui::UICanvas> canvas = std::make_unique<ui::UICanvas>(nullptr);

    surface->on_draw->add_lambda(
        [&](gfx::CommandBuffer* command_buffer)
        {
            canvas->init(ui::UICanvas::Context{
                .draw_pos_x  = 0,
                .draw_pos_y  = 0,
                .draw_width  = window->absolute_width(),
                .draw_height = window->absolute_height(),
            });

            canvas->start_window("this is a test window");
            canvas->label("this is a text yay");
            canvas->end_window();

            canvas->start_window("wintwo");
            canvas->label("this is a text yay");
            canvas->end_window();

            canvas->demo_window();

            canvas->submit(command_buffer);
        });

    while (application::window::Window::get_window_count() > 0)
    {
        surface->render();
        application::get()->next_frame();
    }

    canvas = nullptr;
    delete surface;
    gfx::destroy();
    application::destroy();

    exit(EXIT_SUCCESS);
}
