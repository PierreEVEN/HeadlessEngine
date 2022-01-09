#pragma once
#include "types/type_format.h"

#include "gfx/render_pass_reference.h"

#include <optional>
#include <string>
#include <vector>

namespace gfx
{
struct ClearValue
{
    float    color[4];
    float    depth;
    uint32_t stencil;
};

class RenderPass
{
    friend class Surface;

  public:
    struct Config
    {
        struct Attachment
        {
            std::string               attachment_name = "none";
            ETypeFormat               image_format    = ETypeFormat::R8G8B8A8_UNORM;
            bool                      shader_readable = true;
            std::optional<ClearValue> clear_value;
        };

        std::string               pass_name = "none";
        std::vector<Attachment>   color_attachments;
        std::optional<Attachment> depth_attachment;
    };

    static RenderPass* declare(const Config& frame_graph_config)
    {
        return declare_internal(frame_graph_config, false);
    }
    static RenderPass* find(const RenderPassID& render_pass_name);
    static void        destroy_passes();
    virtual ~RenderPass() = default;

    [[nodiscard]] const Config& get_config() const
    {
        return config;
    }

    [[nodiscard]] bool is_present_pass() const
    {
        return present_pass;
    }

    [[nodiscard]] const RenderPassID& get_id() const
    {
        return render_pass_id;
    }

  protected:
    RenderPass(const Config& frame_graph_config, bool in_present_pass);

  private:
    static RenderPass* declare_internal(const Config& frame_graph_config, bool present_pass);
    RenderPassID       render_pass_id;
    const Config       config;
    bool               present_pass;
};

} // namespace gfx
