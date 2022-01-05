#pragma

#include "gfx/render_pass.h"

#include <vulkan/vulkan.hpp>

#include "vulkan/vk_unit.h"

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
    virtual ~RenderPass_VK() override;

    void begin(CommandBuffer* command_buffer);
    void end(CommandBuffer* command_buffer);

    [[nodiscard]]VkRenderPass get() const
    {
        return render_pass;
    }

protected:
    void init() override;

private:
    VkRenderPass                          render_pass;
    SwapchainImageResource<VkFramebuffer> framebuffers;
    bool                                  is_present_pass = false;
};
} // namespace gfx::vulkan