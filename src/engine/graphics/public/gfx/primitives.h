#pragma once

#include "gfx/Mesh.h"

namespace gfx::primitive
{
std::shared_ptr<Mesh> cube(float width = 1.0);
std::shared_ptr<Mesh> ico_sphere(float diameter = 1, uint32_t subdivision = 1);
std::shared_ptr<Mesh> uv_sphere(float diameter = 1, uint32_t subdivision_x = 5, uint32_t subdivisions_y = 5);
}
