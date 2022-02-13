#pragma once

#include "gfx/Mesh.h"

namespace gfx::primitive
{
std::shared_ptr<Mesh> cube(float x = 1.f, float y = 1.f, float z = 1.f);
std::shared_ptr<Mesh> ico_sphere(float radius = 1, uint32_t subdivision = 1);
std::shared_ptr<Mesh> uv_sphere(float radius = 1, uint32_t subdivision_x = 12, uint32_t subdivision_y = 3);
}
