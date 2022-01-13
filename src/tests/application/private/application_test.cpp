


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


    application::inputs::ActionMapping test_action(application::inputs::EButtons::Keyboard_LeftControl, {application::inputs::EButtons::Keyboard_B});
    test_action.value.value_changed.add_lambda(
        []([[maybe_unused]] bool value)
        {
            LOG_INFO("ACTION : %d", value);
        });

    application::inputs::AxisMapping test_axis(application::inputs::EAxis::Mouse_X, {application::inputs::EButtons::Keyboard_B});
    test_axis.value.value_changed.add_lambda(
        []([[maybe_unused]] float value)
        {
            LOG_INFO("AXIS : %f", value);
        });

    LOG_INFO("value = %d", *test_action.value);

    /**
     * 2° declare some windows with surfaces (surfaces are layers that allow rendering images from the gfx backend onto application window)
     */
    create_window(application::window::WindowConfig{.name = application::get_full_name(), .window_style = application::window::EWindowStyle::WINDOWED});
    while (application::window::Window::get_window_count() > 0)
    {
        application::get()->next_frame();
    }
    
    application::destroy();

    exit(EXIT_SUCCESS);
}
