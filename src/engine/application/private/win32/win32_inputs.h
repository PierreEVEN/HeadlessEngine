#pragma once

#include <cstdint>

namespace application::inputs::win32
{
void input_axis(uint32_t axis_code, int64_t w_param, int64_t l_param);
void press_key(uint64_t key_code, bool extended, uint32_t scan_code);
void update_mouse_buttons(uint64_t mouse_state);
void release_key(uint64_t key_code, bool extended, uint32_t scan_code);
} // namespace application::inputs::win32