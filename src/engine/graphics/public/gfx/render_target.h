#pragma once
#include "texture.h"
#include "view.h"

namespace gfx
{
class RenderTarget
{
  public:
    void update(const View& render_view);

    RenderTarget(uint32_t width, uint32_t height, uint32_t depth);

private:
    std::shared_ptr<Texture> texture_target;
};

} // namespace gfx