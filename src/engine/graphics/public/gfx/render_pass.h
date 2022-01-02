#pragma once
#include "gfx/render_target.h"

#include "gfx/command_buffer.h"
#include "gfx/texture.h"

#include <memory>
#include <string>
#include <vector>

namespace gfx
{
struct ClearValue
{
    float clear_color[4];
};

struct RenderPassConfig
{
    struct Attachment
    {
        std::string               attachment_name = "none";
        EImageFormat              image_format    = EImageFormat::RGBA_UNORM_8;
        bool                      shader_readable = true;
        std::optional<ClearValue> clear_value;
    };

    std::string               pass_name = "none";
    std::vector<Attachment>   color_attachments;
    std::optional<Attachment> depth_attachment;
};

class RenderPass
{
  public:
    static std::shared_ptr<RenderPass> create(uint32_t framebuffer_width, uint32_t framebuffer_height, const RenderPassConfig& frame_graph_config);
    virtual ~RenderPass() = default;


    void add_child(const std::shared_ptr<RenderPass>& render_pass);

    void draw_pass(CommandBuffer* command_buffer);

    void generate()
    {
        if (is_generated)
            return;
        is_generated = true;

        for (const auto& child : children)
            child->generate();

        for (const auto& child : children)
        {
            for (auto& image : child->resource_render_target)
                available_images.emplace_back(image.get());
            for (const auto& image : child->available_images)
                available_images.emplace_back(image);
        }
    }

    void set_framebuffer_images(const std::vector<std::shared_ptr<Texture>>& images);
    void generate_framebuffer_images();

  protected:
    RenderPass(uint32_t framebuffer_width, uint32_t framebuffer_height, const RenderPassConfig& frame_graph_config);

    virtual void init()                               = 0;
    virtual void begin(CommandBuffer* command_buffer) = 0;
    virtual void end(CommandBuffer* command_buffer)   = 0;

    const uint32_t         width;
    const uint32_t         height;
    const RenderPassConfig                config;
    bool                                  is_generated = false;
    std::vector<std::shared_ptr<Texture>> resource_render_target;

  private:

    std::vector<RenderPass*>                 parents;
    std::vector<std::shared_ptr<RenderPass>> children;
    std::vector<Texture*>                    available_images;

    bool present_pass;
};
} // namespace gfx
