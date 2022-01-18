#pragma once

#include "gfx/buffer.h"
#include "gfx/physical_device.h"
#include "gfx/command_buffer.h"
#include "gfx/surface.h"
#include "gfx/master_material.h"
#include "gfx/material_instance.h"
#include "gfx/render_pass.h"
#include "gfx/Mesh.h"
#include "gfx/texture.h"
#include "gfx/view.h"


namespace gfx
{

void init();

void next_frame();

void destroy();

} // namespace gfx