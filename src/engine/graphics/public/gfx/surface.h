#pragma once

#include "application/window.h"
#include "gfx/render_pass.h"

namespace gfx
{
class CommandBuffer;

class Surface
{
  public:
    static Surface* create_surface(application::window::Window* container);
    virtual ~Surface() = default;

    [[nodiscard]] virtual application::window::Window* get_container() const = 0;
    virtual void                                       render()              = 0;

    void add_child(const std::shared_ptr<RenderPass>& render_pass) const
    {
        main_render_pass->add_child(render_pass);
    }

  protected:
      
    Surface() = default;

    std::unique_ptr<RenderPass> main_render_pass;
};
} // namespace gfx
