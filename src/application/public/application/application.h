#pragma once
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
    Application()          = default;
    virtual ~Application() = default;

    void register_monitor_internal(const Monitor& monitor);
    virtual void on_register_internal() = 0;
  protected:
    std::vector<Monitor> available_monitors;
};

void         create();
void         destroy();
Application* get();

} // namespace application
