#pragma once

#include "gfx/render_pass_instance.h"
#include "application/window.h"

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

    void link_dependency(const std::shared_ptr<RenderPassInstance>& render_pass) const
    {
        main_render_pass->link_dependency(render_pass);
    }

    void build_framegraph();

  protected:
      
    Surface() = default;

    std::shared_ptr<RenderPassInstance> main_render_pass;
};
} // namespace gfx
