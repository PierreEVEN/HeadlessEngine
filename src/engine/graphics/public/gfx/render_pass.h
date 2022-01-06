#pragma once
#include "types/type_format.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace gfx
{
struct ClearValue
{
    float clear_color[4];
};

class RenderPass
{
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

    static RenderPass* declare(const Config& frame_graph_config, bool present_pass = false);
    static RenderPass* find(const std::string& render_pass_name);
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

  protected:
    RenderPass(const Config& frame_graph_config);

  private:
    const Config config;
    bool         present_pass;
};

} // namespace gfx
