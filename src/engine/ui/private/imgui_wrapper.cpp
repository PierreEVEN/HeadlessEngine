#include "imgui_wrapper.h"

#include "application/application.h"
#include "application/inputs/input_codes.h"
#include "application/inputs/input_manager.h"

namespace ui
{

static const char* get_clipboard_text(void* user_data)
{
    ImGuiWrapper* wrapper   = static_cast<ImGuiWrapper*>(user_data);
    wrapper->clipboard_text = reinterpret_cast<char*>(application::get()->get_clipboard_data().data());
    return wrapper->clipboard_text.c_str();
}
static void set_clipboard_text([[maybe_unused]] void* user_data, const char* text)
{
    application::get()->set_clipboard_data(std::vector<uint8_t>(text, text + strlen(text) + 1));
}

ImGuiWrapper::ImGuiWrapper(gfx::Surface* surface)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.BackendPlatformUserData != NULL)
        LOG_FATAL("Already initialized a platform backend!");

    io.BackendPlatformUserData = this;
    io.BackendPlatformName     = "imgui_impl_internal";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;      // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;       // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports; // We can create multi-viewports on the Platform side (optional)
    // io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)

    io.KeyMap[ImGuiKey_Tab]         = static_cast<int>(application::inputs::EButtons::Keyboard_Tab);
    io.KeyMap[ImGuiKey_LeftArrow]   = static_cast<int>(application::inputs::EButtons::Keyboard_Left);
    io.KeyMap[ImGuiKey_RightArrow]  = static_cast<int>(application::inputs::EButtons::Keyboard_Right);
    io.KeyMap[ImGuiKey_UpArrow]     = static_cast<int>(application::inputs::EButtons::Keyboard_Up);
    io.KeyMap[ImGuiKey_DownArrow]   = static_cast<int>(application::inputs::EButtons::Keyboard_Down);
    io.KeyMap[ImGuiKey_PageUp]      = static_cast<int>(application::inputs::EButtons::Keyboard_PageUp);
    io.KeyMap[ImGuiKey_PageDown]    = static_cast<int>(application::inputs::EButtons::Keyboard_PageDown);
    io.KeyMap[ImGuiKey_Home]        = static_cast<int>(application::inputs::EButtons::Keyboard_Home);
    io.KeyMap[ImGuiKey_End]         = static_cast<int>(application::inputs::EButtons::Keyboard_End);
    io.KeyMap[ImGuiKey_Insert]      = static_cast<int>(application::inputs::EButtons::Keyboard_Insert);
    io.KeyMap[ImGuiKey_Delete]      = static_cast<int>(application::inputs::EButtons::Keyboard_Delete);
    io.KeyMap[ImGuiKey_Backspace]   = static_cast<int>(application::inputs::EButtons::Keyboard_Backspace);
    io.KeyMap[ImGuiKey_Space]       = static_cast<int>(application::inputs::EButtons::Keyboard_Space);
    io.KeyMap[ImGuiKey_Enter]       = static_cast<int>(application::inputs::EButtons::Keyboard_Enter);
    io.KeyMap[ImGuiKey_Escape]      = static_cast<int>(application::inputs::EButtons::Keyboard_Escape);
    io.KeyMap[ImGuiKey_KeyPadEnter] = static_cast<int>(application::inputs::EButtons::Keyboard_Enter);
    io.KeyMap[ImGuiKey_A]           = static_cast<int>(application::inputs::EButtons::Keyboard_A);
    io.KeyMap[ImGuiKey_C]           = static_cast<int>(application::inputs::EButtons::Keyboard_C);
    io.KeyMap[ImGuiKey_V]           = static_cast<int>(application::inputs::EButtons::Keyboard_V);
    io.KeyMap[ImGuiKey_X]           = static_cast<int>(application::inputs::EButtons::Keyboard_X);
    io.KeyMap[ImGuiKey_Y]           = static_cast<int>(application::inputs::EButtons::Keyboard_Y);
    io.KeyMap[ImGuiKey_Z]           = static_cast<int>(application::inputs::EButtons::Keyboard_Z);

    io.SetClipboardTextFn = set_clipboard_text;
    io.GetClipboardTextFn = get_clipboard_text;
    io.ClipboardUserData  = this;

    ImGuiViewport* main_viewport  = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = surface;
}

static void ImGui_ImplGlfw_UpdateMonitors()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Monitors.resize(0);
    for (int n = 0; n < monitors_count; n++)
    {
        ImGuiPlatformMonitor monitor;
        monitor.MainPos = monitor.WorkPos = ImVec2((float)x, (float)y);
        monitor.MainSize = monitor.WorkSize = ImVec2((float)vid_mode->width, (float)vid_mode->height);
        monitor.DpiScale                    = x_scale;
        platform_io.Monitors.push_back(monitor);
    }
}
static void ImGui_ImplGlfw_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    ImGuiPlatformIO& platform_io  = ImGui::GetPlatformIO();
    for (int n = 0; n < platform_io.Viewports.Size; n++)
    {
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
        {
            // Hide cursor
        }
        else
        {
            // Select cursor
        }
    }
}

void ImGuiWrapper::begin_frame()
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
    if (bd->WantUpdateMonitors) //@TODO : bind to the right imgui event
        ImGui_ImplGlfw_UpdateMonitors();

    // Setup time step
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);

    // Update mouse
    io.MouseDown[0] = application::inputs::Key(application::inputs::EButtons::Mouse_Left).get_bool_value();
    io.MouseDown[1] = application::inputs::Key(application::inputs::EButtons::Mouse_Right).get_bool_value();
    io.MouseDown[2] = application::inputs::Key(application::inputs::EButtons::Mouse_Middle).get_bool_value();
    io.MouseDown[3] = application::inputs::Key(application::inputs::EButtons::Mouse_1).get_bool_value();
    io.MouseDown[4]         = application::inputs::Key(application::inputs::EButtons::Mouse_2).get_bool_value();
    io.MouseHoveredViewport = 0;
    io.MousePos = ImVec2(application::inputs::Key(application::inputs::EAxis::Mouse_X).get_float_value(), application::inputs::Key(application::inputs::EAxis::Mouse_Y).get_float_value());

    ImGui_ImplGlfw_UpdateMouseCursor();
}
} // namespace ui
