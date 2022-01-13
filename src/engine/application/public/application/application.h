#pragma once
#include <chrono>
#include <string>
#include <vector>

namespace application
{

[[nodiscard]] std::string get_name();
[[nodiscard]] std::string get_version();
[[nodiscard]] std::string get_full_name();
[[nodiscard]] std::string get_engine_name();
[[nodiscard]] std::string get_engine_version();
[[nodiscard]] std::string get_engine_full_name();

[[nodiscard]] int get_version_major();
[[nodiscard]] int get_version_minor();
[[nodiscard]] int get_version_patch();

[[nodiscard]] int get_engine_version_major();
[[nodiscard]] int get_engine_version_minor();
[[nodiscard]] int get_engine_version_patch();

struct Monitor
{
    uint32_t dpi_x = 0;
    uint32_t dpi_y = 0;

    uint32_t pos_x  = 0;
    uint32_t pos_y  = 0;
    uint32_t width  = 0;
    uint32_t height = 0;

    uint32_t work_pos_x  = 0;
    uint32_t work_pos_y  = 0;
    uint32_t work_width  = 0;
    uint32_t work_height = 0;
};

class Application
{
  public:
    Application();
    virtual ~Application() = default;

    using AppHandle = uint64_t;

    void              register_monitor_internal(const Monitor& monitor);
    virtual void      on_register_internal()    = 0;
    virtual AppHandle get_platform_app_handle() = 0;

    virtual void                 set_clipboard_data(const std::vector<uint8_t>& clipboard_data) = 0;
    virtual std::vector<uint8_t> get_clipboard_data()                                           = 0;

    [[nodiscard]] double time() const
    {
        return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_time).count()) * 1.0e-9;
    }
    [[nodiscard]] double delta_time() const
    {
        return time_delta;
    }

    void next_frame();

  protected:
    std::vector<Monitor>                  available_monitors;
    std::chrono::steady_clock::time_point start_time;
    double                                time_delta;
    double                                last_time;
};

void         create();
void         destroy();
Application* get();

} // namespace application
