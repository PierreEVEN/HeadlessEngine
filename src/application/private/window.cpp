#include "application/window.h"

#include "win32/win32_window.h"

#include <vector>

namespace application::window
{
static std::vector<Window*> registered_windows;

uint32_t Window::get_window_count()
{
    return static_cast<uint32_t>(registered_windows.size());
}

Window* Window::get_window(uint32_t index)
{
    return registered_windows[index];
}

void Window::register_window(Window* new_window)
{
    registered_windows.emplace_back(new_window);
}

void Window::unregister_window(Window* destroyed_window)
{
    registered_windows.erase(std::ranges::find(registered_windows, destroyed_window));
}

Window* create_window(const WindowConfig& config)
{
    return new win32::Window_Win32(config);
}

void destroy_window(Window* window)
{
    delete window;
}
} // namespace application::window