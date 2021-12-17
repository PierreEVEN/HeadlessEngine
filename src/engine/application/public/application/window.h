#pragma once

#include <cstdint>
#include <string>

namespace application::window
{
enum class EWindowStyle
{
    WINDOWED,
    BORDERLESS,
    FULLSCREEN,
};

struct WindowConfig
{
    std::string  name         = "none";
    uint32_t     width        = 800;
    uint32_t     height       = 600;
    int32_t      pos_x        = 0;
    int32_t      pos_y        = 0;
    float        opacity      = 1;
    EWindowStyle window_style = EWindowStyle::WINDOWED;
};

class Window
{
  public:
    Window(const WindowConfig& new_config) : config(new_config)
    {
    }
    virtual ~Window() = default;

    virtual void set_name(const std::string& new_name)      = 0;
    virtual void set_size(uint32_t width, uint32_t height)  = 0;
    virtual void set_position(int32_t pos_x, int32_t pos_y) = 0;
    virtual void set_opacity(float alpha)                   = 0;
    virtual void show()                                     = 0;
    virtual void update()                                   = 0;

    static uint32_t get_window_count();

    static Window*    get_window(uint32_t index);
    
    using WindowHandle = uint64_t;
    virtual WindowHandle get_platform_window_handle() = 0;

  protected:
    static void  register_window(Window* new_window);
    static void  unregister_window(Window* destroyed_window);
    WindowConfig config;
};

Window* create_window(const WindowConfig& config);
void    destroy_window(Window* window);
} // namespace application::window
