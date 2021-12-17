#pragma once

#include "application/window.h"
#include <Windows.h>

namespace application::window::win32
{

class Window_Win32 : public Window
{
  public:
    Window_Win32(const WindowConfig& config);
    virtual ~Window_Win32();
    void set_name(const std::string& new_name) override;
    void set_size(uint32_t width, uint32_t height) override;
    void set_position(int32_t pos_x, int32_t pos_y) override;
    void set_opacity(float alpha) override;
    void show() override;

    LRESULT CALLBACK window_behaviour(uint32_t in_msg, WPARAM in_wparam, LPARAM in_lparam);
    static Window_Win32* find_window_by_handle(HWND handle);
    void update() override;
    static void register_window_class(HINSTANCE handle);

    WindowHandle get_platform_window_handle() override
    {
        return reinterpret_cast<WindowHandle>(window_handle);
    }
  private:
    HWND         window_handle;
    DWORD        style;
    DWORD        ex_style;
    bool         has_called_destroy;
};

} // namespace application::window::win32
