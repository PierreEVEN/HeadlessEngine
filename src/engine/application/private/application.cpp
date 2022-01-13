#include "application/application.h"

#if APP_USE_WIN32
#include "win32/win32_application.h"
#endif

#include "application/window.h"

#include <cpputils/logger.hpp>
#include <cpputils/stringutils.hpp>

namespace application
{
static Application* application = nullptr;

[[nodiscard]] std::string get_name()
{
    return "no application name";
}
[[nodiscard]] std::string get_version()
{
    return stringutils::format("%d.%d.%d", get_version_major(), get_version_minor(), get_version_patch());
}
[[nodiscard]] std::string get_full_name()
{
    return get_name() + " " + get_version();
}
[[nodiscard]] std::string get_engine_name()
{
    return "no engine name";
}
[[nodiscard]] std::string get_engine_version()
{
    return stringutils::format("%d.%d.%d", get_engine_version_major(), get_engine_version_minor(), get_engine_version_patch());
}
[[nodiscard]] std::string get_engine_full_name()
{
    return get_engine_name() + " " + get_engine_version();
}

[[nodiscard]] int get_version_major()
{
    return 0;
}
[[nodiscard]] int get_version_minor()
{
    return 0;
}
[[nodiscard]] int get_version_patch()
{
    return 0;
}

[[nodiscard]] int get_engine_version_major()
{
    return 0;
}
[[nodiscard]] int get_engine_version_minor()
{
    return 0;
}
[[nodiscard]] int get_engine_version_patch()
{
    return 0;
}

Application::Application() : time_delta(1 / 60.0), last_time(0)
{
    start_time = std::chrono::steady_clock::now();
}

void Application::register_monitor_internal(const Monitor& monitor)
{
    available_monitors.emplace_back(monitor);
}

void Application::next_frame()
{
    const auto current_time = time();
    time_delta              = current_time - last_time;
    last_time               = current_time;

    for (uint32_t i = 0; i < window::Window::get_window_count(); ++i)
        window::Window::get_window(i)->update();
}

void create()
{
    if (application)
        LOG_FATAL("application is already created");

#if APP_USE_WIN32
    application = new win32::Application_Win32();
    application->on_register_internal();
#else
    LOG_ERROR("application backend is not supported");
#endif
}
void destroy()
{
    if (!application)
        return;
    delete application;
    application = nullptr;
}

Application* get()
{
    if (!application)
        LOG_FATAL("application should be created first");
    return application;
}
} // namespace application