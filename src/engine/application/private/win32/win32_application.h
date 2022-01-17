#pragma once

#include "application/application.h"
#include <Windows.h>
#include <cpputils/logger.hpp>

namespace application::win32
{

inline void PrintWin32Error(DWORD error)
{
    if (error == 0)
        return;

    LPSTR messageBuffer = nullptr;
    ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, NULL);
    std::ostringstream error_number;
    error_number << error;
    LOG_FATAL("Win32 error %s : %s", error_number.str().c_str(), messageBuffer);
    //::LocalFree(messageBuffer);
}

#define WIN_CHECK(result)                                        \
    do                                                           \
    {                                                            \
        if ((result) == 0)                                       \
        {                                                        \
            application::win32::PrintWin32Error(GetLastError()); \
        }                                                        \
    } while (false)

class Application_Win32 final : public Application
{
  public:
    Application_Win32(HINSTANCE handle = nullptr);
    virtual ~Application_Win32();

    [[nodiscard]] HINSTANCE get_handle() const
    {
        return application_handle;
    }
    void                 set_clipboard_data(const std::vector<uint8_t>& clipboard_data) override;
    std::vector<uint8_t> get_clipboard_data() override;

    void on_register_internal() override;

    AppHandle get_platform_app_handle() override
    {
        return reinterpret_cast<AppHandle>(application_handle);
    }
    void set_cursor(ECursorStyle cursor_style) override;
  private:
    HCURSOR   load_cursor(LPCSTR cursor);
    HINSTANCE application_handle;
};

} // namespace application::win32