#pragma once

#include "gfx/gfx.h"

#include <imgui.h>

namespace ui
{


class ImGuiWrapper
{
public:
    ImGuiWrapper(gfx::Surface* surface);

    void draw_to_buffer(gfx::CommandBuffer* command_buffer) {}

    void begin_frame();

    std::string clipboard_text;
};
}
