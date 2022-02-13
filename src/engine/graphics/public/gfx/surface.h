#pragma once

#include "application/window.h"
#include "gfx/render_pass_instance.h"

namespace gfx
{
class CommandBuffer;

class Surface
{
  public:
    static Surface* create_surface(const std::string& name, application::window::Window* container);
    virtual ~Surface() = default;

    [[nodiscard]] virtual application::window::Window* get_container() const = 0;
    [[nodiscard]] virtual bool                         prepare_next_frame()  = 0;
    virtual void                                       render()              = 0;

    void        link_dependency(const std::shared_ptr<RenderPassInstance>& render_pass) const;
    void        build_framegraph();
    OnDrawPass* on_draw = nullptr;

  protected:
    Surface(const std::string& name, application::window::Window* container) : window_container(container), surface_name(name)
    {
    }

    application::window::Window*        window_container;
    const std::string&                  surface_name;
    std::shared_ptr<RenderPassInstance> main_render_pass;
};
} // namespace gfx
