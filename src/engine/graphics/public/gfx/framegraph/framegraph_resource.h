#pragma once
#include "gfx/render_target.h"

#include <memory>
#include <string>
#include <vector>

namespace gfx
{
struct FrameGraphResourceConfig
{
    struct Attachment
    {
        std::string attachment_name;
    };

    std::string             pass_name;
    std::vector<Attachment> attachments;
};

class FrameGraphResource
{
  public:
    using Handle = size_t;

    void add_child(const std::shared_ptr<FrameGraphResource>& render_pass);

    void render()
    {
    }

    void generate()
    {
        if (is_generated)
            return;
        is_generated = true;

        for (const auto& child : children)
            child->generate();

        generate_render_targets();

        for (const auto& child : children)
        {
            for (auto& image : child->resource_render_target)
                available_images.emplace_back(&image);
            for (const auto& image : child->available_images)
                available_images.emplace_back(image);
        }
    }

  private:
    void generate_render_targets()
    {
    }

    bool                                             is_generated = false;
    std::vector<RenderTarget>                        resource_render_target;
    std::vector<FrameGraphResource*>                 parents;
    std::vector<std::shared_ptr<FrameGraphResource>> children;
    std::vector<RenderTarget*>                       available_images;
};
} // namespace gfx
