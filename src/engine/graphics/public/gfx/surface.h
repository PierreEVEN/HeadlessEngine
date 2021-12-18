#pragma once

#include "application/window.h"

namespace gfx
{
class CommandBuffer;

class Surface
{
  public:
    static Surface* create_surface(application::window::Window* container);
    virtual ~Surface() = default;

    virtual void submit_command_buffer(const CommandBuffer* command_buffer) = 0;

  protected:
    Surface() = default;

  private:
};
} // namespace gfx
