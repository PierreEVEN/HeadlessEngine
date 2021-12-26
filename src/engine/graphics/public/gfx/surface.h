#pragma once

#include "render_target.h"
#include "application/window.h"

namespace gfx
{
class CommandBuffer;

class Surface
{
  public:
    static Surface* create_surface(application::window::Window* container);
    virtual ~Surface() = default;

    virtual void display(RenderTarget& render_target) = 0;

  protected:
    Surface() = default;

  private:
};
} // namespace gfx
