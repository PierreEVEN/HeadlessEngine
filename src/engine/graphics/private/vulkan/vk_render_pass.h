#pragma

#include "gfx/render_pass.h"

#include <vulkan/vulkan.hpp>

namespace gfx
{
class Surface;
}

namespace gfx::vulkan
{

class RenderPass_VK final : public RenderPass
{
  public:
    RenderPass_VK(const Config& frame_graph_config);
    ~RenderPass_VK() override;

    [[nodiscard]] const VkRenderPass& get() const
    {
        return render_pass;
    }

  protected:
  private:
    VkRenderPass render_pass;
};
} // namespace gfx::vulkan