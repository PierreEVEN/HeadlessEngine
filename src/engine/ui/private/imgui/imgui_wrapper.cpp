#include "imgui_wrapper.h"

#include "application/application.h"
#include "application/inputs/input_codes.h"
#include "application/inputs/input_manager.h"

#include <imgui.h>

namespace ui
{
static ImGuiContext*                 imgui_context = nullptr;
static std::shared_ptr<gfx::Texture> font_texture;
static gfx::Buffer*                  vertex_buffer;
static gfx::Buffer*                  index_buffer;

static const char* get_clipboard_text(void* user_data)
{
    return "";
    /*
    ImGuiWrapper* wrapper   = static_cast<ImGuiWrapper*>(user_data);
    wrapper->clipboard_text = reinterpret_cast<char*>(application::get()->get_clipboard_data().data());
    return wrapper->clipboard_text.c_str();
    */
}
static void set_clipboard_text([[maybe_unused]] void* user_data, const char* text)
{
    application::get()->set_clipboard_data(std::vector<uint8_t>(text, text + strlen(text) + 1));
}

void ImGuiWrapper::init_internal()
{
    imgui_context = ImGui::CreateContext();
    IMGUI_CHECKVERSION();

    ImGuiIO& io = ImGui::GetIO();
    if (io.BackendPlatformUserData != nullptr)
        LOG_FATAL("Imgui has already been initialized !");

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.BackendPlatformUserData = nullptr;
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

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    style.WindowRounding     = 0;
    style.ScrollbarRounding  = 0;
    style.TabRounding        = 0;
    style.WindowBorderSize   = 1;
    style.PopupBorderSize    = 1;
    style.WindowTitleAlign.x = 0.5f;
    style.FramePadding.x     = 6.f;
    style.FramePadding.y     = 6.f;
    style.WindowPadding.x    = 4.f;
    style.WindowPadding.y    = 4.f;
    style.GrabMinSize        = 16.f;
    style.ScrollbarSize      = 20.f;
    style.IndentSpacing      = 30.f;

    ImGuiViewport* main_viewport  = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = nullptr;

    // Create font texture
    uint8_t* pixels;
    int      width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    font_texture    = gfx::Texture::create(width, height, gfx::TextureParameter{});
    io.Fonts->TexID = font_texture.get();
}

void ImGuiWrapper::destroy()
{
    ImGui::DestroyContext();
    font_texture = nullptr;
}

void ImGuiWrapper::begin_frame(const UICanvas::Context& context)
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for start_window resizing)
    io.DisplaySize             = ImVec2(static_cast<float>(context.draw_width), static_cast<float>(context.draw_height));
    io.DisplayFramebufferScale = ImVec2(1.0, 1.0);

    io.DeltaTime = static_cast<float>(application::get()->delta_time());

    // Update mouse
    io.MouseDown[0]         = application::inputs::Key(application::inputs::EButtons::Mouse_Left).get_bool_value();
    io.MouseDown[1]         = application::inputs::Key(application::inputs::EButtons::Mouse_Right).get_bool_value();
    io.MouseDown[2]         = application::inputs::Key(application::inputs::EButtons::Mouse_Middle).get_bool_value();
    io.MouseDown[3]         = application::inputs::Key(application::inputs::EButtons::Mouse_1).get_bool_value();
    io.MouseDown[4]         = application::inputs::Key(application::inputs::EButtons::Mouse_2).get_bool_value();
    io.MouseHoveredViewport = 0;
    io.MousePos             = ImVec2(application::inputs::Key(application::inputs::EAxis::Mouse_X).get_float_value(), application::inputs::Key(application::inputs::EAxis::Mouse_Y).get_float_value());

    ImGui::NewFrame();
}

void ImGuiWrapper::submit_frame(gfx::CommandBuffer* command_buffer)
{
    ImGui::EndFrame();

    const auto* draw_data = ImGui::GetDrawData();
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    const int fb_width  = static_cast<int>(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    const int fb_height = static_cast<int>(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0 || draw_data->TotalVtxCount == 0)
        return;

    /**
     * BUILD VERTEX BUFFERS
     */
    // Get pointer to buffer data
    vertex_buffer->set_data_lambda(
        [&](void* vertex_ptr)
        {
            auto* vertex_data = static_cast<uint8_t*>(vertex_ptr);
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                memcpy(vertex_data, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                vertex_data += cmd_list->VtxBuffer.Size;
            }
        });

    index_buffer->set_data_lambda(
        [&](void* index_ptr)
        {
            auto index_data = static_cast<uint8_t*>(index_ptr);
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                memcpy(index_data, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                index_data += cmd_list->IdxBuffer.Size;
            }
        });

    /**
     * PREPARE MATERIALS
     */

    auto         pipeline  = material_instance->get_material_base()->get_pipeline(render_pass);
    VkDeviceSize offsets[] = {0};
    material_instance->bind_material(render_context);
    vkCmdBindVertexBuffers(render_context.command_buffer, 0, 1, &vertex_buffer[image_index].get_buffer(), offsets);
    vkCmdBindIndexBuffer(render_context.command_buffer, index_buffer[image_index].get_buffer(), 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

    float scale[2] = {
        2.0f / draw_data->DisplaySize.x,
        -2.0f / draw_data->DisplaySize.y,
    };
    float translate[2] = {
        -1.0f - draw_data->DisplayPos.x * scale[0],
        1.0f - draw_data->DisplayPos.y * scale[1],
    };

    vkCmdPushConstants(render_context.command_buffer, *pipeline->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2, scale);
    vkCmdPushConstants(render_context.command_buffer, *pipeline->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);

    /**
     * Draw meshs
     */

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
                pcmd->UserCallback(cmd_list, pcmd);
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Negative offsets are illegal for vkCmdSetScissor
                    if (clip_rect.x < 0.0f)
                        clip_rect.x = 0.0f;
                    if (clip_rect.y < 0.0f)
                        clip_rect.y = 0.0f;

                    // Apply scissor/clipping rectangle
                    command_buffer->set_scissor(gfx::Scissor{
                        .offset_x = static_cast<int32_t>(clip_rect.x),
                        .offset_y = static_cast<int32_t>(clip_rect.y),
                        .width    = static_cast<uint32_t>(clip_rect.z - clip_rect.x),
                        .height   = static_cast<uint32_t>(clip_rect.w - clip_rect.y),
                    });

                    // Bind descriptorset with font or user texture
                    VkDescriptorSet desc_set[1] = {static_cast<VkDescriptorSet>(pcmd->TextureId)};
                    if (!pcmd->TextureId)
                        desc_set[1] = {static_cast<VkDescriptorSet>(
                            TAssetPtr<ATexture>("default_texture")->get_imgui_handle(0, *TAssetPtr<AMaterialBase>("imgui_base_material")->get_pipeline("ui_render_pass")->get_descriptor_sets_layouts()))};
                    vkCmdBindDescriptorSets(render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->get_pipeline_layout(), 0, 1, desc_set, 0, NULL);

                    command_buffer->draw_mesh();
                    // Draw
                    vkCmdDrawIndexed(render_context.command_buffer, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
                }
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

ImGuiContext* ImGuiWrapper::get_context()
{
    return imgui_context;
}

} // namespace ui
