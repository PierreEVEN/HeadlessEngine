#pragma once

#include "gfx/render_pass_instance.h"
#include "application/window.h"

namespace gfx
{
class CommandBuffer;

class ISurfaceDrawInterface
{
  public:
    virtual void draw(CommandBuffer*) = 0;
};

template <typename Lambda_T> class TSurfaceDrawInterface : public ISurfaceDrawInterface
{
  public:
    TSurfaceDrawInterface(const Lambda_T& lambda) : lambda_func(lambda)
    {
    }
    void draw(CommandBuffer* cmd) override
    {
        lambda_func(cmd);
    }

  private:
    Lambda_T lambda_func;
};
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

    template <typename Lambda_T> void on_draw(const Lambda_T& lambda)
    {
        draw_interface = std::make_unique<TSurfaceDrawInterface<Lambda_T>>(lambda);
    }
  protected:
      
    Surface() = default;

    std::unique_ptr<ISurfaceDrawInterface> draw_interface;

    std::shared_ptr<RenderPassInstance> main_render_pass;
};
} // namespace gfx
