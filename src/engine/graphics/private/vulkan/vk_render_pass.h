#pragma

#include "vk_descriptor_pool.h"
#include "gfx/render_pass.h"

#include <vulkan/vulkan.h>

namespace gfx
{
class Surface;
}

namespace gfx::vulkan
{

class RenderPass_VK final : public RenderPass
{
  public:
    RenderPass_VK(const Config & frame_graph_config, bool in_present_pass);
    ~RenderPass_VK() override;

    [[nodiscard]] const VkRenderPass& get() const
    {
        return render_pass;
    }

    DescriptorPool_VK& get_descriptor_pool()
    {
        return descriptor_pool;
    }

  protected:
  private:
    DescriptorPool_VK descriptor_pool;
    VkRenderPass render_pass;
};
} // namespace gfx::vulkan