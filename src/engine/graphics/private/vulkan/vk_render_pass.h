#pragma

#include "gfx/framegraph/framegraph_resource.h"

#include <vulkan/vulkan.hpp>

#include "unit.h"

namespace gfx
{
class Surface;
}

namespace gfx::vulkan
{

class RenderPass_VK : public RenderPass
{
    friend Surface;
  public:
    RenderPass_VK(uint32_t framebuffer_width, uint32_t framebuffer_height, const RenderPassConfig& frame_graph_config);

    void begin(CommandBuffer* command_buffer);
    void end(CommandBuffer* command_buffer);
protected:
    void init() override;

private:
    VkRenderPass                          render_pass;
    SwapchainImageResource<VkFramebuffer> framebuffers;
    bool                                  is_present_pass = false;
};
} // namespace gfx::vulkan