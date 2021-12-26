#pragma once
#include "view.h"

namespace gfx
{
class RenderTarget
{
  public:
    void update(const View& render_view);
};

} // namespace gfx