
#include "application/inputs/input_manager.h"

namespace application::inputs
{
std::unique_ptr<InputManager> singleton;

InputManager& InputManager::get()
{
    if (!singleton)
        singleton = std::unique_ptr<InputManager>(new InputManager());
    return *singleton;
}

void InputManager::press_button(EButtons key)
{
    if (!buttons[key].pressed)
        buttons[key].first_pressed = true;
    else
        buttons[key].first_pressed = false;
    buttons[key].pressed = true;
    buttons[key].value_changed.execute();
}

void InputManager::release_button(EButtons key)
{
    buttons[key].first_pressed = false;
    buttons[key].pressed       = false;
    buttons[key].value_changed.execute();
    LOG_INFO("%s", magic_enum::enum_name(key).data());
}

void InputManager::move_axis(EAxis key, float value)
{
    axis[key].value = value;
    axis[key].value_changed.execute();
}

InputManager::InputManager()
{
    for (const auto& entry : magic_enum::enum_entries<EButtons>())
        buttons[entry.first] = {};
    for (const auto& entry : magic_enum::enum_entries<EAxis>())
        axis[entry.first] = {};
}
} // namespace application::inputs