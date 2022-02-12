#pragma once

#include "gfx/command_buffer.h"
#include "gfx/render_pass.h"
#include "gfx/texture.h"

#include <cpputils/eventmanager.hpp>

namespace gfx
{
DECLARE_DELEGATE_MULTICAST(OnDrawPass, CommandBuffer*);

class RenderPassInstance
{
  public:
    static std::shared_ptr<RenderPassInstance> create(uint32_t width, uint32_t height, const RenderPassID& base, const std::vector<std::shared_ptr<Texture>>& images = {});

    virtual ~RenderPassInstance();

    virtual void resize(uint32_t width, uint32_t height, const std::vector<std::shared_ptr<Texture>>& surface_texture = {}) = 0;
    void         draw_pass();

    void link_dependency(const std::shared_ptr<RenderPassInstance>& render_pass);
    void build_framegraph();

    [[nodiscard]] uint32_t get_width() const
    {
        return framebuffer_width;
    }

    [[nodiscard]] uint32_t get_height() const
    {
        return framebuffer_height;
    }

    [[nodiscard]] RenderPass* get_base() const
    {
        return render_pass_base;
    }

    OnDrawPass on_draw_pass;

    [[nodiscard]] const std::vector<std::shared_ptr<Texture>>& get_framebuffer_images() const
    {
        return framebuffers_images;
    }

    [[nodiscard]] CommandBuffer* get_pass_command_buffer() const
    {
        return command_buffer;
    }

  protected:
    RenderPassInstance(uint32_t width, uint32_t height, const RenderPassID& base, const std::vector<std::shared_ptr<Texture>>& images);
    virtual void begin_pass() = 0;
    virtual void submit()     = 0;

    std::vector<std::shared_ptr<RenderPassInstance>> children;
    std::vector<std::shared_ptr<Texture>>            framebuffers_images;
    bool                                             is_generated = false;
    uint32_t                                         framebuffer_width;
    uint32_t                                         framebuffer_height;

    void create_or_recreate_framebuffer_images();

  private:
    CommandBuffer*                        command_buffer = nullptr;
    RenderPass*                           render_pass_base;
    std::vector<RenderPassInstance*>      parents;
    std::vector<Texture*>                 available_images;
};
} // namespace gfx
