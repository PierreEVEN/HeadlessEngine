#pragma once

#include <cstdint>
#include <gfx/gfx.h>

namespace ui
{

class UICanvas
{
  public:
    UICanvas(const std::shared_ptr<gfx::RenderPassInstance>& render_pass);
    ~UICanvas();

    void start_window(const std::string& title);
    void end_window();
    void demo_window();

    void label(const std::string& text);

    struct Context
    {
        uint32_t draw_pos_x;
        uint32_t draw_pos_y;
        uint32_t draw_width;
        uint32_t draw_height;
        uint32_t mouse_pos_x;
        uint32_t mouse_pos_y;
    };

    void init(const Context& context);
    void submit(gfx::CommandBuffer* command_buffer);


  private:
    std::mutex imgui_draw_lock__;
};

} // namespace ui