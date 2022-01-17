#include "ui.h"

#include "imgui/imgui_wrapper.h"

#include <imgui.h>

namespace ui
{
UICanvas::UICanvas([[maybe_unused]] const std::shared_ptr<gfx::RenderPassInstance>& render_pass)
{
    if (ImGuiWrapper::usage_count == 0)
        ImGuiWrapper::init();
    ImGuiWrapper::usage_count++;
}

UICanvas::~UICanvas()
{
    ImGuiWrapper::usage_count--;
    if (ImGuiWrapper::usage_count == 0)
        ImGuiWrapper::destroy();
}

void UICanvas::label(const std::string& text)
{
    ImGui::Text("%s", text.c_str());
}

void UICanvas::start_window(const std::string& title)
{
    ImGui::Begin(title.c_str());
}

void UICanvas::end_window()
{
    ImGui::ShowDemoWindow();
    ImGui::End();
}

void UICanvas::init(const Context& context)
{
    imgui_draw_lock__.lock();
    ImGuiWrapper::begin_frame(context);
}

void UICanvas::submit(gfx::CommandBuffer* command_buffer)
{
    ImGuiWrapper::submit_frame(command_buffer);
    imgui_draw_lock__.unlock();
}
} // namespace ui
