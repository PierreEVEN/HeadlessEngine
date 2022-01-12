#pragma once

#include "gfx/gfx.h"

#include <imgui.h>

namespace ui
{


class ImGuiWrapper
{
public:
    ImGuiWrapper();

    void draw_to_buffer(gfx::CommandBuffer* command_buffer) {}
};
}
