#include "win32_application.h"

#include "application/application.h"
#include "win32_window.h"
#include <ShellScalingApi.h>
#include <cpputils/logger.hpp>
#include <cstdint>
#include <unordered_map>
#include <types/magic_enum.h>

namespace application::win32
{
static std::unordered_map<LPCSTR, HCURSOR> cursor_map;

Application_Win32::Application_Win32(HINSTANCE handle)
{
    application_handle = handle;

    if (application_handle == nullptr)
        application_handle = GetModuleHandle(nullptr);

    window::win32::Window_Win32::register_window_class(application_handle);
}
Application_Win32::~Application_Win32()
{
    ::CoUninitialize();
}

void Application_Win32::on_register_internal()
{
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM data) -> BOOL
        {
            (void)data;

            MONITORINFO win_monitor_info = {sizeof(MONITORINFO)};
            WIN_CHECK(::GetMonitorInfo(monitor, &win_monitor_info));

            UINT dpi_x;
            UINT dpi_y;
            WIN_CHECK(::GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y));

            application::get()->register_monitor_internal({
                .dpi_x       = dpi_x,
                .dpi_y       = dpi_y,
                .pos_x       = static_cast<uint32_t>(win_monitor_info.rcMonitor.left),
                .pos_y       = static_cast<uint32_t>(win_monitor_info.rcMonitor.top),
                .width       = static_cast<uint32_t>(win_monitor_info.rcMonitor.right - win_monitor_info.rcMonitor.left),
                .height      = static_cast<uint32_t>(win_monitor_info.rcMonitor.bottom - win_monitor_info.rcMonitor.top),
                .work_pos_x  = static_cast<uint32_t>(win_monitor_info.rcWork.left),
                .work_pos_y  = static_cast<uint32_t>(win_monitor_info.rcWork.top),
                .work_width  = static_cast<uint32_t>(win_monitor_info.rcWork.right - win_monitor_info.rcWork.left),
                .work_height = static_cast<uint32_t>(win_monitor_info.rcWork.bottom - win_monitor_info.rcWork.top),
            });
            return TRUE;
        },
        reinterpret_cast<LPARAM>(this));
}

void Application_Win32::set_clipboard_data([[maybe_unused]] const std::vector<uint8_t>& clipboard_data)
{
    LOG_ERROR("clipboard is not implemented");
}

HCURSOR Application_Win32::load_cursor(LPCSTR cursor)
{
    if (!cursor_map.contains(cursor))
        WIN_CHECK(cursor_map[cursor] = LoadCursorA(nullptr, cursor));
    return cursor_map[cursor];
}

void Application_Win32::set_cursor(ECursorStyle cursor_style)
{
    HCURSOR cursor = nullptr;
    switch (cursor_style)
    {
    default:
    case ECursorStyle::ARROW:
        cursor = load_cursor(IDC_ARROW);
        break;
    case ECursorStyle::CROSSHAIR:
        cursor = load_cursor(IDC_CROSS);
        break;
    case ECursorStyle::HAND:
        cursor = load_cursor(IDC_HAND);
        break;
    case ECursorStyle::HELP:
        cursor = load_cursor(IDC_HELP);
        break;
    case ECursorStyle::I_BEAM:
        cursor = load_cursor(IDC_IBEAM);
        break;
    case ECursorStyle::SIZE_ALL:
        cursor = load_cursor(IDC_SIZEALL);
        break;
    case ECursorStyle::SIZE_NESW:
        cursor = load_cursor(IDC_SIZENESW);
        break;
    case ECursorStyle::SIZE_NS:
        cursor = load_cursor(IDC_SIZENS);
        break;
    case ECursorStyle::SIZE_NWSE:
        cursor = load_cursor(IDC_SIZENWSE);
        break;
    case ECursorStyle::SIZE_WE:
        cursor = load_cursor(IDC_SIZEWE);
        break;
    case ECursorStyle::VERT_ARROW:
        cursor = load_cursor(IDC_UPARROW);
        break;
    case ECursorStyle::WAIT:
        cursor = load_cursor(IDC_WAIT);
        break;
    case ECursorStyle::NOT_ALLOWED:
        cursor = load_cursor(IDC_NO);
        break;
    }
    SetCursor(cursor);
    //PrintWin32Error(ShowCursor(true));
}

std::vector<uint8_t> Application_Win32::get_clipboard_data()
{
    LOG_ERROR("clipboard is not implemented");
    return {'\0'};
}

} // namespace application::win32