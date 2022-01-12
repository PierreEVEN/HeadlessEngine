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
} // namespace application::inputs