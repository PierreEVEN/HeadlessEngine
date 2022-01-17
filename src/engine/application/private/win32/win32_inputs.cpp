
#include "win32/win32_inputs.h"

#include "application/inputs/input_manager.h"

#include <cpputils/logger.hpp>

#include <Windows.h>
#include <windowsx.h>

namespace application::inputs::win32
{

static uint64_t last_mouse_state = 0;

void input_axis(uint32_t axis_code, int64_t scroll_param, int64_t pos_param)
{
    switch (axis_code)
    {
    case WM_MOUSEMOVE:
        InputManager::get().move_axis(EAxis::Mouse_X, static_cast<float>(GET_X_LPARAM(pos_param)));
        InputManager::get().move_axis(EAxis::Mouse_Y, static_cast<float>(GET_Y_LPARAM(pos_param)));
        break;
    case WM_MOUSEWHEEL:
        InputManager::get().move_axis(EAxis::Scroll_Y, static_cast<float>(GET_Y_LPARAM(scroll_param)));
        break;
    case WM_MOUSEHWHEEL:
        InputManager::get().move_axis(EAxis::Scroll_X, static_cast<float>(GET_X_LPARAM(scroll_param)));
        break;
    default:
        LOG_WARNING("unhandled axis : 0x%x", axis_code);
    }
}
void update_mouse_buttons(uint64_t mouse_state)
{
    if (mouse_state & VK_LBUTTON && !(last_mouse_state & VK_LBUTTON))
        InputManager::get().press_button(EButtons::Mouse_Left);
    if (!(mouse_state & VK_LBUTTON) && last_mouse_state & VK_LBUTTON)
        InputManager::get().release_button(EButtons::Mouse_Left);

    if (mouse_state & VK_RBUTTON && !(last_mouse_state & VK_RBUTTON))
        InputManager::get().press_button(EButtons::Mouse_Right);
    if (!(mouse_state & VK_RBUTTON) && last_mouse_state & VK_RBUTTON)
        InputManager::get().release_button(EButtons::Mouse_Right);

    if (mouse_state & VK_MBUTTON && !(last_mouse_state & VK_MBUTTON))
        InputManager::get().press_button(EButtons::Mouse_Middle);
    if (!(mouse_state & VK_MBUTTON) && last_mouse_state & VK_MBUTTON)
        InputManager::get().release_button(EButtons::Mouse_Middle);

    if (mouse_state & VK_XBUTTON1 && !(last_mouse_state & VK_XBUTTON1))
        InputManager::get().press_button(EButtons::Mouse_1);
    if (!(mouse_state & VK_XBUTTON1) && last_mouse_state & VK_XBUTTON1)
        InputManager::get().release_button(EButtons::Mouse_1);

    if (mouse_state & VK_XBUTTON2 && !(last_mouse_state & VK_XBUTTON2))
        InputManager::get().press_button(EButtons::Mouse_2);
    if (!(mouse_state & VK_XBUTTON2) && last_mouse_state & VK_XBUTTON2)
        InputManager::get().release_button(EButtons::Mouse_2);
    
    last_mouse_state = mouse_state;
}

void press_key(uint64_t key_code, bool extended, uint32_t scan_code)
{
    switch (key_code)
    {
    case 0x00:
    case 0xFF:
        break;
    case VK_BACK:
        InputManager::get().press_button(EButtons::Keyboard_Backspace);
        break;
    case VK_TAB:
        InputManager::get().press_button(EButtons::Keyboard_Tab);
        break;
    case VK_PAUSE:
        InputManager::get().press_button(EButtons::Keyboard_Pause);
        break;
    case VK_RETURN:
        InputManager::get().press_button(EButtons::Keyboard_Enter);
        break;
    case VK_SHIFT:
        InputManager::get().press_button(MapVirtualKey(scan_code, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT ? EButtons::Keyboard_RightShift : EButtons::Keyboard_LeftShift);
        break;
    case VK_ESCAPE:
        InputManager::get().press_button(EButtons::Keyboard_Escape);
        break;
    case VK_SPACE:
        InputManager::get().press_button(EButtons::Keyboard_Space);
        break;
    case VK_PRIOR:
        InputManager::get().press_button(EButtons::Keyboard_PageUp);
        break;
    case VK_NEXT:
        InputManager::get().press_button(EButtons::Keyboard_PageDown);
        break;
    case VK_END:
        InputManager::get().press_button(EButtons::Keyboard_End);
        break;
    case VK_HOME:
        InputManager::get().press_button(EButtons::Keyboard_Home);
        break;
    case VK_LEFT:
        InputManager::get().press_button(EButtons::Keyboard_Left);
        break;
    case VK_RIGHT:
        InputManager::get().press_button(EButtons::Keyboard_Right);
        break;
    case VK_UP:
        InputManager::get().press_button(EButtons::Keyboard_Up);
        break;
    case VK_DOWN:
        InputManager::get().press_button(EButtons::Keyboard_Down);
        break;
    case VK_PRINT:
        InputManager::get().press_button(EButtons::Keyboard_Print);
        break;
    case VK_SNAPSHOT:
        InputManager::get().press_button(EButtons::Keyboard_PrintScreen);
        break;
    case VK_INSERT:
        InputManager::get().press_button(EButtons::Keyboard_Insert);
        break;
    case VK_DELETE:
        InputManager::get().press_button(EButtons::Keyboard_Delete);
        break;
    case VK_HELP:
        InputManager::get().press_button(EButtons::Keyboard_Help);
        break;
    case 0x30:
        InputManager::get().press_button(EButtons::Keyboard_0);
        break;
    case 0x31:
        InputManager::get().press_button(EButtons::Keyboard_1);
        break;
    case 0x32:
        InputManager::get().press_button(EButtons::Keyboard_2);
        break;
    case 0x33:
        InputManager::get().press_button(EButtons::Keyboard_3);
        break;
    case 0x34:
        InputManager::get().press_button(EButtons::Keyboard_4);
        break;
    case 0x35:
        InputManager::get().press_button(EButtons::Keyboard_5);
        break;
    case 0x36:
        InputManager::get().press_button(EButtons::Keyboard_6);
        break;
    case 0x37:
        InputManager::get().press_button(EButtons::Keyboard_7);
        break;
    case 0x38:
        InputManager::get().press_button(EButtons::Keyboard_8);
        break;
    case 0x39:
        InputManager::get().press_button(EButtons::Keyboard_9);
        break;
    case 0x41:
        InputManager::get().press_button(EButtons::Keyboard_A);
        break;
    case 0x42:
        InputManager::get().press_button(EButtons::Keyboard_B);
        break;
    case 0x43:
        InputManager::get().press_button(EButtons::Keyboard_C);
        break;
    case 0x44:
        InputManager::get().press_button(EButtons::Keyboard_D);
        break;
    case 0x45:
        InputManager::get().press_button(EButtons::Keyboard_E);
        break;
    case 0x46:
        InputManager::get().press_button(EButtons::Keyboard_F);
        break;
    case 0x47:
        InputManager::get().press_button(EButtons::Keyboard_G);
        break;
    case 0x48:
        InputManager::get().press_button(EButtons::Keyboard_H);
        break;
    case 0x49:
        InputManager::get().press_button(EButtons::Keyboard_I);
        break;
    case 0x4A:
        InputManager::get().press_button(EButtons::Keyboard_J);
        break;
    case 0x4B:
        InputManager::get().press_button(EButtons::Keyboard_K);
        break;
    case 0x4C:
        InputManager::get().press_button(EButtons::Keyboard_L);
        break;
    case 0x4D:
        InputManager::get().press_button(EButtons::Keyboard_M);
        break;
    case 0x4E:
        InputManager::get().press_button(EButtons::Keyboard_N);
        break;
    case 0x4F:
        InputManager::get().press_button(EButtons::Keyboard_O);
        break;
    case 0x50:
        InputManager::get().press_button(EButtons::Keyboard_P);
        break;
    case 0x51:
        InputManager::get().press_button(EButtons::Keyboard_Q);
        break;
    case 0x52:
        InputManager::get().press_button(EButtons::Keyboard_R);
        break;
    case 0x53:
        InputManager::get().press_button(EButtons::Keyboard_S);
        break;
    case 0x54:
        InputManager::get().press_button(EButtons::Keyboard_T);
        break;
    case 0x55:
        InputManager::get().press_button(EButtons::Keyboard_U);
        break;
    case 0x56:
        InputManager::get().press_button(EButtons::Keyboard_V);
        break;
    case 0x57:
        InputManager::get().press_button(EButtons::Keyboard_W);
        break;
    case 0x58:
        InputManager::get().press_button(EButtons::Keyboard_X);
        break;
    case 0x59:
        InputManager::get().press_button(EButtons::Keyboard_Y);
        break;
    case 0x5A:
        InputManager::get().press_button(EButtons::Keyboard_Z);
        break;
    case VK_LWIN:
        InputManager::get().press_button(EButtons::Keyboard_LeftWin);
        break;
    case VK_RWIN:
        InputManager::get().press_button(EButtons::Keyboard_RightWin);
        break;
    case VK_SLEEP:
        InputManager::get().press_button(EButtons::Keyboard_Sleep);
        break;
    case VK_NUMPAD0:
        InputManager::get().press_button(EButtons::Keyboard_Num0);
        break;
    case VK_NUMPAD1:
        InputManager::get().press_button(EButtons::Keyboard_Num1);
        break;
    case VK_NUMPAD2:
        InputManager::get().press_button(EButtons::Keyboard_Num2);
        break;
    case VK_NUMPAD3:
        InputManager::get().press_button(EButtons::Keyboard_Num3);
        break;
    case VK_NUMPAD4:
        InputManager::get().press_button(EButtons::Keyboard_Num4);
        break;
    case VK_NUMPAD5:
        InputManager::get().press_button(EButtons::Keyboard_Num5);
        break;
    case VK_NUMPAD6:
        InputManager::get().press_button(EButtons::Keyboard_Num6);
        break;
    case VK_NUMPAD7:
        InputManager::get().press_button(EButtons::Keyboard_Num7);
        break;
    case VK_NUMPAD8:
        InputManager::get().press_button(EButtons::Keyboard_Num8);
        break;
    case VK_NUMPAD9:
        InputManager::get().press_button(EButtons::Keyboard_Num9);
        break;
    case VK_MULTIPLY:
        InputManager::get().press_button(EButtons::Keyboard_NumMultiply);
        break;
    case VK_DECIMAL:
        InputManager::get().press_button(EButtons::Keyboard_NumDelete);
        break;
    case VK_ADD:
        InputManager::get().press_button(EButtons::Keyboard_NumAdd);
        break;
    case VK_SUBTRACT:
        InputManager::get().press_button(EButtons::Keyboard_NumSubtract);
        break;
    case VK_DIVIDE:
        InputManager::get().press_button(EButtons::Keyboard_NumDivide);
        break;
    case VK_F1:
        InputManager::get().press_button(EButtons::Keyboard_F1);
        break;
    case VK_F2:
        InputManager::get().press_button(EButtons::Keyboard_F2);
        break;
    case VK_F3:
        InputManager::get().press_button(EButtons::Keyboard_F3);
        break;
    case VK_F4:
        InputManager::get().press_button(EButtons::Keyboard_F4);
        break;
    case VK_F5:
        InputManager::get().press_button(EButtons::Keyboard_F5);
        break;
    case VK_F6:
        InputManager::get().press_button(EButtons::Keyboard_F6);
        break;
    case VK_F7:
        InputManager::get().press_button(EButtons::Keyboard_F7);
        break;
    case VK_F8:
        InputManager::get().press_button(EButtons::Keyboard_F8);
        break;
    case VK_F9:
        InputManager::get().press_button(EButtons::Keyboard_F9);
        break;
    case VK_F10:
        InputManager::get().press_button(EButtons::Keyboard_F10);
        break;
    case VK_F11:
        InputManager::get().press_button(EButtons::Keyboard_F11);
        break;
    case VK_F12:
        InputManager::get().press_button(EButtons::Keyboard_F12);
        break;
    case VK_F13:
        InputManager::get().press_button(EButtons::Keyboard_F13);
        break;
    case VK_F14:
        InputManager::get().press_button(EButtons::Keyboard_F14);
        break;
    case VK_F15:
        InputManager::get().press_button(EButtons::Keyboard_F15);
        break;
    case VK_F16:
        InputManager::get().press_button(EButtons::Keyboard_F16);
        break;
    case VK_F17:
        InputManager::get().press_button(EButtons::Keyboard_F17);
        break;
    case VK_F18:
        InputManager::get().press_button(EButtons::Keyboard_F18);
        break;
    case VK_F19:
        InputManager::get().press_button(EButtons::Keyboard_F19);
        break;
    case VK_F20:
        InputManager::get().press_button(EButtons::Keyboard_F20);
        break;
    case VK_F21:
        InputManager::get().press_button(EButtons::Keyboard_F21);
        break;
    case VK_F22:
        InputManager::get().press_button(EButtons::Keyboard_F22);
        break;
    case VK_F23:
        InputManager::get().press_button(EButtons::Keyboard_F23);
        break;
    case VK_F24:
        InputManager::get().press_button(EButtons::Keyboard_F24);
        break;
    case VK_NUMLOCK:
        InputManager::get().press_button(EButtons::Keyboard_NumLock);
        break;
    case VK_SCROLL:
        InputManager::get().press_button(EButtons::Keyboard_ScrollStop);
        break;
    case VK_LSHIFT:
        InputManager::get().press_button(EButtons::Keyboard_LeftShift);
        break;
    case VK_RSHIFT:
        InputManager::get().press_button(EButtons::Keyboard_RightShift);
        break;
    case VK_LCONTROL:
        InputManager::get().press_button(EButtons::Keyboard_LeftControl);
        break;
    case VK_RCONTROL:
        InputManager::get().press_button(EButtons::Keyboard_RightControl);
        break;
    case VK_APPS:
        InputManager::get().press_button(EButtons::Keyboard_Apps);
        break;
    case VK_LMENU:
        InputManager::get().press_button(EButtons::Keyboard_LeftMenu);
        break;
    case VK_RMENU:
        InputManager::get().press_button(EButtons::Keyboard_RightMenu);
        break;
    case VK_VOLUME_MUTE:
        InputManager::get().press_button(EButtons::Keyboard_VolumeMute);
        break;
    case VK_VOLUME_UP:
        InputManager::get().press_button(EButtons::Keyboard_VolumeUp);
        break;
    case VK_VOLUME_DOWN:
        InputManager::get().press_button(EButtons::Keyboard_VolumeDown);
        break;
    case VK_MEDIA_NEXT_TRACK:
        InputManager::get().press_button(EButtons::Keyboard_MediaNextTrack);
        break;
    case VK_MEDIA_PREV_TRACK:
        InputManager::get().press_button(EButtons::Keyboard_MediaPrevTrack);
        break;
    case VK_MEDIA_PLAY_PAUSE:
        InputManager::get().press_button(EButtons::Keyboard_MediaPlayPause);
        break;
    case VK_MEDIA_STOP:
        InputManager::get().press_button(EButtons::Keyboard_MediaStop);
        break;
    case VK_OEM_PLUS:
        InputManager::get().press_button(EButtons::Keyboard_Add);
        break;
    case VK_OEM_7:
        InputManager::get().press_button(EButtons::Keyboard_Power);
        break;
    case VK_CAPITAL:
        InputManager::get().press_button(EButtons::Keyboard_CapsLock);
        break;
    case VK_CONTROL:
        InputManager::get().press_button(extended ? EButtons::Keyboard_RightControl : EButtons::Keyboard_LeftControl);
        break;
    case VK_MENU:
        InputManager::get().press_button(EButtons::Keyboard_Alt);
        break;
    case VK_OEM_8:
        InputManager::get().press_button(EButtons::Keyboard_Exclamation);
        break;
    case VK_OEM_2:
        InputManager::get().press_button(EButtons::Keyboard_Colon);
        break;
    case VK_OEM_PERIOD:
        InputManager::get().press_button(EButtons::Keyboard_Period);
        break;
    case VK_OEM_COMMA:
        InputManager::get().press_button(EButtons::Keyboard_Comma);
        break;
    case VK_OEM_4:
        InputManager::get().press_button(EButtons::Keyboard_LeftBracket);
        break;
    case VK_OEM_3:
        InputManager::get().press_button(EButtons::Keyboard_Tilde);
        break;
    case VK_OEM_1:
        InputManager::get().press_button(EButtons::Keyboard_Semicolon);
        break;
    case VK_OEM_6:
        InputManager::get().press_button(EButtons::Keyboard_RightBracket);
        break;
    case VK_OEM_5:
        InputManager::get().press_button(EButtons::Keyboard_LeftBracket);
        break;
    case VK_OEM_102:
        InputManager::get().press_button(EButtons::Keyboard_BackSlash);
        break;
    default:
        LOG_WARNING("press unknown 0x%x", key_code);
    }
}

void release_key(uint64_t key_code, bool extended, uint32_t scan_code)
{
    switch (key_code)
    {
    case 0x00:
    case 0xFF:
        break;
    case VK_BACK:
        InputManager::get().release_button(EButtons::Keyboard_Backspace);
        break;
    case VK_TAB:
        InputManager::get().release_button(EButtons::Keyboard_Tab);
        break;
    case VK_PAUSE:
        InputManager::get().release_button(EButtons::Keyboard_Pause);
        break;
    case VK_RETURN:
        InputManager::get().release_button(EButtons::Keyboard_Enter);
        break;
    case VK_SHIFT:
        InputManager::get().release_button(MapVirtualKey(scan_code, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT ? EButtons::Keyboard_RightShift : EButtons::Keyboard_LeftShift);
        break;
    case VK_ESCAPE:
        InputManager::get().release_button(EButtons::Keyboard_Escape);
        break;
    case VK_SPACE:
        InputManager::get().release_button(EButtons::Keyboard_Space);
        break;
    case VK_PRIOR:
        InputManager::get().release_button(EButtons::Keyboard_PageUp);
        break;
    case VK_NEXT:
        InputManager::get().release_button(EButtons::Keyboard_PageDown);
        break;
    case VK_END:
        InputManager::get().release_button(EButtons::Keyboard_End);
        break;
    case VK_HOME:
        InputManager::get().release_button(EButtons::Keyboard_Home);
        break;
    case VK_LEFT:
        InputManager::get().release_button(EButtons::Keyboard_Left);
        break;
    case VK_RIGHT:
        InputManager::get().release_button(EButtons::Keyboard_Right);
        break;
    case VK_UP:
        InputManager::get().release_button(EButtons::Keyboard_Up);
        break;
    case VK_DOWN:
        InputManager::get().release_button(EButtons::Keyboard_Down);
        break;
    case VK_PRINT:
        InputManager::get().release_button(EButtons::Keyboard_Print);
        break;
    case VK_SNAPSHOT:
        InputManager::get().release_button(EButtons::Keyboard_PrintScreen);
        break;
    case VK_INSERT:
        InputManager::get().release_button(EButtons::Keyboard_Insert);
        break;
    case VK_DELETE:
        InputManager::get().release_button(EButtons::Keyboard_Delete);
        break;
    case VK_HELP:
        InputManager::get().release_button(EButtons::Keyboard_Help);
        break;
    case 0x30:
        InputManager::get().release_button(EButtons::Keyboard_0);
        break;
    case 0x31:
        InputManager::get().release_button(EButtons::Keyboard_1);
        break;
    case 0x32:
        InputManager::get().release_button(EButtons::Keyboard_2);
        break;
    case 0x33:
        InputManager::get().release_button(EButtons::Keyboard_3);
        break;
    case 0x34:
        InputManager::get().release_button(EButtons::Keyboard_4);
        break;
    case 0x35:
        InputManager::get().release_button(EButtons::Keyboard_5);
        break;
    case 0x36:
        InputManager::get().release_button(EButtons::Keyboard_6);
        break;
    case 0x37:
        InputManager::get().release_button(EButtons::Keyboard_7);
        break;
    case 0x38:
        InputManager::get().release_button(EButtons::Keyboard_8);
        break;
    case 0x39:
        InputManager::get().release_button(EButtons::Keyboard_9);
        break;
    case 0x41:
        InputManager::get().release_button(EButtons::Keyboard_A);
        break;
    case 0x42:
        InputManager::get().release_button(EButtons::Keyboard_B);
        break;
    case 0x43:
        InputManager::get().release_button(EButtons::Keyboard_C);
        break;
    case 0x44:
        InputManager::get().release_button(EButtons::Keyboard_D);
        break;
    case 0x45:
        InputManager::get().release_button(EButtons::Keyboard_E);
        break;
    case 0x46:
        InputManager::get().release_button(EButtons::Keyboard_F);
        break;
    case 0x47:
        InputManager::get().release_button(EButtons::Keyboard_G);
        break;
    case 0x48:
        InputManager::get().release_button(EButtons::Keyboard_H);
        break;
    case 0x49:
        InputManager::get().release_button(EButtons::Keyboard_I);
        break;
    case 0x4A:
        InputManager::get().release_button(EButtons::Keyboard_J);
        break;
    case 0x4B:
        InputManager::get().release_button(EButtons::Keyboard_K);
        break;
    case 0x4C:
        InputManager::get().release_button(EButtons::Keyboard_L);
        break;
    case 0x4D:
        InputManager::get().release_button(EButtons::Keyboard_M);
        break;
    case 0x4E:
        InputManager::get().release_button(EButtons::Keyboard_N);
        break;
    case 0x4F:
        InputManager::get().release_button(EButtons::Keyboard_O);
        break;
    case 0x50:
        InputManager::get().release_button(EButtons::Keyboard_P);
        break;
    case 0x51:
        InputManager::get().release_button(EButtons::Keyboard_Q);
        break;
    case 0x52:
        InputManager::get().release_button(EButtons::Keyboard_R);
        break;
    case 0x53:
        InputManager::get().release_button(EButtons::Keyboard_S);
        break;
    case 0x54:
        InputManager::get().release_button(EButtons::Keyboard_T);
        break;
    case 0x55:
        InputManager::get().release_button(EButtons::Keyboard_U);
        break;
    case 0x56:
        InputManager::get().release_button(EButtons::Keyboard_V);
        break;
    case 0x57:
        InputManager::get().release_button(EButtons::Keyboard_W);
        break;
    case 0x58:
        InputManager::get().release_button(EButtons::Keyboard_X);
        break;
    case 0x59:
        InputManager::get().release_button(EButtons::Keyboard_Y);
        break;
    case 0x5A:
        InputManager::get().release_button(EButtons::Keyboard_Z);
        break;
    case VK_LWIN:
        InputManager::get().release_button(EButtons::Keyboard_LeftWin);
        break;
    case VK_RWIN:
        InputManager::get().release_button(EButtons::Keyboard_RightWin);
        break;
    case VK_SLEEP:
        InputManager::get().release_button(EButtons::Keyboard_Sleep);
        break;
    case VK_NUMPAD0:
        InputManager::get().release_button(EButtons::Keyboard_Num0);
        break;
    case VK_NUMPAD1:
        InputManager::get().release_button(EButtons::Keyboard_Num1);
        break;
    case VK_NUMPAD2:
        InputManager::get().release_button(EButtons::Keyboard_Num2);
        break;
    case VK_NUMPAD3:
        InputManager::get().release_button(EButtons::Keyboard_Num3);
        break;
    case VK_NUMPAD4:
        InputManager::get().release_button(EButtons::Keyboard_Num4);
        break;
    case VK_NUMPAD5:
        InputManager::get().release_button(EButtons::Keyboard_Num5);
        break;
    case VK_NUMPAD6:
        InputManager::get().release_button(EButtons::Keyboard_Num6);
        break;
    case VK_NUMPAD7:
        InputManager::get().release_button(EButtons::Keyboard_Num7);
        break;
    case VK_NUMPAD8:
        InputManager::get().release_button(EButtons::Keyboard_Num8);
        break;
    case VK_NUMPAD9:
        InputManager::get().release_button(EButtons::Keyboard_Num9);
        break;
    case VK_MULTIPLY:
        InputManager::get().release_button(EButtons::Keyboard_NumMultiply);
        break;
    case VK_DECIMAL:
        InputManager::get().release_button(EButtons::Keyboard_NumDelete);
        break;
    case VK_ADD:
        InputManager::get().release_button(EButtons::Keyboard_NumAdd);
        break;
    case VK_SUBTRACT:
        InputManager::get().release_button(EButtons::Keyboard_NumSubtract);
        break;
    case VK_DIVIDE:
        InputManager::get().release_button(EButtons::Keyboard_NumDivide);
        break;
    case VK_F1:
        InputManager::get().release_button(EButtons::Keyboard_F1);
        break;
    case VK_F2:
        InputManager::get().release_button(EButtons::Keyboard_F2);
        break;
    case VK_F3:
        InputManager::get().release_button(EButtons::Keyboard_F3);
        break;
    case VK_F4:
        InputManager::get().release_button(EButtons::Keyboard_F4);
        break;
    case VK_F5:
        InputManager::get().release_button(EButtons::Keyboard_F5);
        break;
    case VK_F6:
        InputManager::get().release_button(EButtons::Keyboard_F6);
        break;
    case VK_F7:
        InputManager::get().release_button(EButtons::Keyboard_F7);
        break;
    case VK_F8:
        InputManager::get().release_button(EButtons::Keyboard_F8);
        break;
    case VK_F9:
        InputManager::get().release_button(EButtons::Keyboard_F9);
        break;
    case VK_F10:
        InputManager::get().release_button(EButtons::Keyboard_F10);
        break;
    case VK_F11:
        InputManager::get().release_button(EButtons::Keyboard_F11);
        break;
    case VK_F12:
        InputManager::get().release_button(EButtons::Keyboard_F12);
        break;
    case VK_F13:
        InputManager::get().release_button(EButtons::Keyboard_F13);
        break;
    case VK_F14:
        InputManager::get().release_button(EButtons::Keyboard_F14);
        break;
    case VK_F15:
        InputManager::get().release_button(EButtons::Keyboard_F15);
        break;
    case VK_F16:
        InputManager::get().release_button(EButtons::Keyboard_F16);
        break;
    case VK_F17:
        InputManager::get().release_button(EButtons::Keyboard_F17);
        break;
    case VK_F18:
        InputManager::get().release_button(EButtons::Keyboard_F18);
        break;
    case VK_F19:
        InputManager::get().release_button(EButtons::Keyboard_F19);
        break;
    case VK_F20:
        InputManager::get().release_button(EButtons::Keyboard_F20);
        break;
    case VK_F21:
        InputManager::get().release_button(EButtons::Keyboard_F21);
        break;
    case VK_F22:
        InputManager::get().release_button(EButtons::Keyboard_F22);
        break;
    case VK_F23:
        InputManager::get().release_button(EButtons::Keyboard_F23);
        break;
    case VK_F24:
        InputManager::get().release_button(EButtons::Keyboard_F24);
        break;
    case VK_NUMLOCK:
        InputManager::get().release_button(EButtons::Keyboard_NumLock);
        break;
    case VK_SCROLL:
        InputManager::get().release_button(EButtons::Keyboard_ScrollStop);
        break;
    case VK_LSHIFT:
        InputManager::get().release_button(EButtons::Keyboard_LeftShift);
        break;
    case VK_RSHIFT:
        InputManager::get().release_button(EButtons::Keyboard_RightShift);
        break;
    case VK_LCONTROL:
        InputManager::get().release_button(EButtons::Keyboard_LeftControl);
        break;
    case VK_RCONTROL:
        InputManager::get().release_button(EButtons::Keyboard_RightControl);
        break;
    case VK_APPS:
        InputManager::get().release_button(EButtons::Keyboard_Apps);
        break;
    case VK_LMENU:
        InputManager::get().release_button(EButtons::Keyboard_LeftMenu);
        break;
    case VK_RMENU:
        InputManager::get().release_button(EButtons::Keyboard_RightMenu);
        break;
    case VK_VOLUME_MUTE:
        InputManager::get().release_button(EButtons::Keyboard_VolumeMute);
        break;
    case VK_VOLUME_UP:
        InputManager::get().release_button(EButtons::Keyboard_VolumeUp);
        break;
    case VK_VOLUME_DOWN:
        InputManager::get().release_button(EButtons::Keyboard_VolumeDown);
        break;
    case VK_MEDIA_NEXT_TRACK:
        InputManager::get().release_button(EButtons::Keyboard_MediaNextTrack);
        break;
    case VK_MEDIA_PREV_TRACK:
        InputManager::get().release_button(EButtons::Keyboard_MediaPrevTrack);
        break;
    case VK_MEDIA_PLAY_PAUSE:
        InputManager::get().release_button(EButtons::Keyboard_MediaPlayPause);
        break;
    case VK_MEDIA_STOP:
        InputManager::get().release_button(EButtons::Keyboard_MediaStop);
        break;
    case VK_OEM_PLUS:
        InputManager::get().release_button(EButtons::Keyboard_Add);
        break;
    case VK_OEM_7:
        InputManager::get().release_button(EButtons::Keyboard_Power);
        break;
    case VK_CAPITAL:
        InputManager::get().release_button(EButtons::Keyboard_CapsLock);
        break;
    case VK_CONTROL:
        InputManager::get().release_button(extended ? EButtons::Keyboard_RightControl : EButtons::Keyboard_LeftControl);
        break;
    case VK_MENU:
        InputManager::get().release_button(EButtons::Keyboard_Alt);
        break;
    case VK_OEM_8:
        InputManager::get().release_button(EButtons::Keyboard_Exclamation);
        break;
    case VK_OEM_2:
        InputManager::get().release_button(EButtons::Keyboard_Colon);
        break;
    case VK_OEM_PERIOD:
        InputManager::get().release_button(EButtons::Keyboard_Period);
        break;
    case VK_OEM_COMMA:
        InputManager::get().release_button(EButtons::Keyboard_Comma);
        break;
    case VK_OEM_4:
        InputManager::get().release_button(EButtons::Keyboard_LeftBracket);
        break;
    case VK_OEM_3:
        InputManager::get().release_button(EButtons::Keyboard_Tilde);
        break;
    case VK_OEM_1:
        InputManager::get().release_button(EButtons::Keyboard_Semicolon);
        break;
    case VK_OEM_6:
        InputManager::get().release_button(EButtons::Keyboard_RightBracket);
        break;
    case VK_OEM_5:
        InputManager::get().release_button(EButtons::Keyboard_LeftBracket);
        break;
    case VK_OEM_102:
        InputManager::get().release_button(EButtons::Keyboard_BackSlash);
        break;
    default:
        LOG_WARNING("release unknown 0x%x", key_code);
    }
}
} // namespace application::inputs::win32
