#pragma once

#include "gfx/buffer.h"
#include "gfx/drawing.h"
#include "gfx/physical_device.h"
#include "gfx/command_buffer.h"
#include "gfx/surface.h"
#include "gfx/render_target.h"
#include "gfx/render_pass.h"


namespace gfx
{

void init();

void next_frame();

void destroy();

} // namespace gfx