
#include "win32_window.h"

#include "application/application.h"
#include "win32_application.h"

namespace application::window::win32
{

Window_Win32::Window_Win32(const WindowConfig& config) : Window(config)
{
    has_called_destroy = false;
    style              = NULL;
    ex_style           = NULL;

    if (config.window_style == EWindowStyle::WINDOWED)
    {
        style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    }
    else
    {
        style |= WS_POPUP;
        style |= WS_MAXIMIZE;
    }

    ex_style |= WS_EX_LAYERED;
    ex_style |= WS_EX_ACCEPTFILES; // DRAG&DROP
    ex_style |= WS_EX_APPWINDOW | WS_EX_TOPMOST;

    RECT initial_area = {0, 0, static_cast<LONG>(config.width), static_cast<LONG>(config.height)};
    WIN_CHECK(::AdjustWindowRectEx(&initial_area, style, FALSE, ex_style));

    window_handle = ::CreateWindowEx(ex_style, "headless_engine_window_class", config.name.c_str(), style, config.pos_x, config.pos_y, initial_area.right - initial_area.left, initial_area.bottom - initial_area.top,
                                     nullptr, nullptr, dynamic_cast<application::win32::Application_Win32*>(application::get())->get_handle(), nullptr);
    WIN_CHECK(window_handle);

    WIN_CHECK(::SetLayeredWindowAttributes(window_handle, 0, static_cast<uint8_t>(config.opacity * 255), LWA_ALPHA));
    WIN_CHECK(::ShowWindow(window_handle, config.window_style == EWindowStyle::BORDERLESS ? SW_MAXIMIZE : SW_SHOW));

    /*
    //@TODO : add fullscreen mode
    DEVMODE fullscreenSettings;

    EnumDisplaySettings(NULL, 0, &fullscreenSettings);
    fullscreenSettings.dmPelsWidth        = 1920;
    fullscreenSettings.dmPelsHeight       = 1080;
    fullscreenSettings.dmBitsPerPel       = 32;
    fullscreenSettings.dmDisplayFrequency = 165;
    fullscreenSettings.dmFields           = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

    SetWindowLongPtr(window_handle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
    SetWindowLongPtr(window_handle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(window_handle, HWND_TOPMOST, 0, 0, fullscreenSettings.dmPelsWidth, fullscreenSettings.dmPelsHeight, SWP_SHOWWINDOW);
    ChangeDisplaySettings(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
    ShowWindow(window_handle, SW_MAXIMIZE);
    */

    register_window(this);
}

Window_Win32::~Window_Win32()
{
    has_called_destroy = true;
    WIN_CHECK(::DestroyWindow(window_handle));
    unregister_window(this);
}

void Window_Win32::set_name(const std::string& new_name)
{
    config.name = new_name;
    WIN_CHECK(::SetWindowText(window_handle, new_name.c_str()));
}

void Window_Win32::set_size(uint32_t width, uint32_t height)
{
    config.width  = width;
    config.height = height;

    RECT initial_area = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    WIN_CHECK(::AdjustWindowRectEx(&initial_area, style, FALSE, ex_style));
    WIN_CHECK(::SetWindowPos(window_handle, nullptr, config.pos_x, config.pos_y, initial_area.right - initial_area.left, initial_area.bottom - initial_area.top, 0));
}

void Window_Win32::set_position(int32_t pos_x, int32_t pos_y)
{
    config.pos_x = pos_x;
    config.pos_y = pos_y;
    WIN_CHECK(::SetWindowPos(window_handle, nullptr, pos_x, pos_y, config.width, config.height, 0));
}

void Window_Win32::set_opacity(float alpha)
{
    config.opacity = alpha;
    WIN_CHECK(::SetLayeredWindowAttributes(window_handle, 0, static_cast<uint8_t>(config.opacity * 255), LWA_ALPHA));
}

void Window_Win32::show()
{
    WIN_CHECK(::ShowWindow(window_handle, SW_SHOW));
}

LRESULT Window_Win32::window_behaviour(uint32_t in_msg, WPARAM in_wparam, LPARAM in_lparam)
{
    if (has_called_destroy)
        return ::DefWindowProc(window_handle, in_msg, in_wparam, in_lparam);

    switch (in_msg)
    {
    case WM_DESTROY:
        LOG_INFO("destroy !!");
        destroy_window(this);
        return 0;
        break;
    case WM_PAINT:
        return 0;
    case WM_SIZE:
    {
        config.width  = LOWORD(in_lparam);
        config.height = HIWORD(in_lparam);
        return 0;
    }
    case WM_MOVE:
    {
        config.pos_x = LOWORD(in_lparam);
        config.pos_y = HIWORD(in_lparam);
        return 0;
    }
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        LOG_INFO("button");
        return TRUE;
    }

    return ::DefWindowProc(window_handle, in_msg, in_wparam, in_lparam);
}

Window_Win32* Window_Win32::find_window_by_handle(HWND handle)
{
    for (uint32_t i = 0; i < get_window_count(); ++i)
    {
        Window_Win32* window = dynamic_cast<Window_Win32*>(get_window(i));
        if (window->window_handle == handle)
            return window;
    }
    return nullptr;
}

void Window_Win32::update()
{
    ::UpdateWindow(window_handle);
    MSG msg;
    if (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Window_Win32::register_window_class(HINSTANCE handle)
{
    WIN_CHECK(::CoInitialize(nullptr));

    WNDCLASS application_window_class = {
        .style       = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        .lpfnWndProc = [](HWND in_hwnd, uint32_t in_msg, WPARAM wparam, LPARAM lparam) -> LRESULT {
            if (window::win32::Window_Win32* window = window::win32::Window_Win32::find_window_by_handle(in_hwnd))
                return window->window_behaviour(in_msg, wparam, lparam);

            const LRESULT result = ::DefWindowProc(in_hwnd, in_msg, wparam, lparam);

            WIN_CHECK(result);

            return result;
        },
        .hInstance     = handle,
        .lpszClassName = "headless_engine_window_class",
    };

    WIN_CHECK(::RegisterClass(&application_window_class));
}
} // namespace application::window::win32