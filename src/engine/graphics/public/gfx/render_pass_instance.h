#pragma once

#include "gfx/command_buffer.h"
#include "gfx/render_pass.h"
#include "gfx/texture.h"

namespace gfx
{
class RenderPassInstance
{
  public:
    static std::shared_ptr<RenderPassInstance> create(uint32_t width, uint32_t height, RenderPass* base, const std::optional<std::vector<std::shared_ptr<Texture>>>& images = {});

    virtual ~RenderPassInstance() = default;

    virtual void resize(uint32_t width, uint32_t height) = 0;
    void         draw_pass(CommandBuffer* command_buffer);

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

  protected:
    RenderPassInstance(uint32_t width, uint32_t height, RenderPass* base, const std::optional<std::vector<std::shared_ptr<Texture>>>& images);
    virtual void begin(CommandBuffer* command_buffer) = 0;
    virtual void end(CommandBuffer* command_buffer)   = 0;

    std::vector<std::shared_ptr<Texture>> framebuffers_images;

  private:
    RenderPass*                                      render_pass_base;
    uint32_t                                         framebuffer_width;
    uint32_t                                         framebuffer_height;
    std::vector<RenderPassInstance*>                 parents;
    std::vector<std::shared_ptr<RenderPassInstance>> children;
    bool                                             is_generated = false;
    std::vector<Texture*>                            available_images;
};
} // namespace gfx
