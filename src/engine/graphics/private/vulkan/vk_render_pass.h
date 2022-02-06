#pragma

#include "gfx/render_pass.h"
#include "gfx/resource/gpu_resource.h"
#include "vk_descriptor_pool.h"

#include <vulkan/vulkan.h>

namespace gfx
{
class Surface;
}

namespace gfx::vulkan
{

class RenderPassResource_VK final
{
  public:
    RenderPassResource_VK(const std::string& name, const RenderPass::Config& render_pass_config, bool in_present_pass);
    ~RenderPassResource_VK();

    VkRenderPass render_pass;
};

class RenderPass_VK final : public RenderPass
{
  public:
    RenderPass_VK(const Config& frame_graph_config, bool in_present_pass);
    ~RenderPass_VK() override = default;

    [[nodiscard]] const TGpuHandle<RenderPassResource_VK>& get() const
    {
        return render_pass;
    }

    DescriptorPool_VK& get_descriptor_pool()
    {
        return descriptor_pool;
    }

  private:
    DescriptorPool_VK                 descriptor_pool;
    TGpuHandle<RenderPassResource_VK> render_pass;
};
} // namespace gfx::vulkan