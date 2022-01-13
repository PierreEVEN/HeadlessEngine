#pragma once

#include "gfx/gfx.h"
#include "ui.h"

#include <imgui.h>

namespace ui
{

class ImGuiWrapper final
{
  public:
    ImGuiWrapper()  = delete;
    ~ImGuiWrapper() = delete;

    __inline static void init()
    {
        init_internal();
        make_context_current();
    }
    static void destroy();

    __inline static void make_context_current()
    {
        ImGui::SetCurrentContext(get_context());
    }

    static void begin_frame(const UICanvas::Context& context);
    static void submit_frame(gfx::CommandBuffer* command_buffer);

    std::string clipboard_text;
    
    inline static int usage_count = 0;

  private:
    static void          init_internal();
    static ImGuiContext* get_context();
};
} // namespace ui
