


#include "application/application.h"
#include "application/window.h"
#include "application/inputs/input_mapping.h"

#include <cpputils/logger.hpp>

int main()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    /**
     * 1° initialize the application and the gfx backend
     */
    application::create();


    application::inputs::ActionMapping toto(application::inputs::EAxis::Mouse_X, {application::inputs::EButtons::Keyboard_B});

    toto.value.value_changed.add_lambda(
        []([[maybe_unused]] bool value)
        {
            LOG_INFO("key changed");
        });

    LOG_INFO("value = %d", *toto.value);

    /**
     * 2° declare some windows with surfaces (surfaces are layers that allow rendering images from the gfx backend onto application window)
     */
    auto* window_1  = create_window(application::window::WindowConfig{.name = application::get_full_name(), .window_style = application::window::EWindowStyle::WINDOWED});
    while (application::window::Window::get_window_count() > 0)
    {
        for (uint32_t i = 0; i < application::window::Window::get_window_count(); ++i)
        {
            application::window::Window::get_window(i)->update();
        }
    }
    
    application::destroy();

    exit(EXIT_SUCCESS);
}
