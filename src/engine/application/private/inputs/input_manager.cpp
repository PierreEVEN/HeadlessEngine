
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
    LOG_INFO("press %s", magic_enum::enum_name(key).data());
}

void InputManager::release_button(EButtons key)
{
    LOG_INFO("release %s", magic_enum::enum_name(key).data());
}

void InputManager::move_axis(EAxis key, int value)
{
    LOG_INFO("axis_move %s %d", magic_enum::enum_name(key).data(), value);
}
} // namespace application::inputs