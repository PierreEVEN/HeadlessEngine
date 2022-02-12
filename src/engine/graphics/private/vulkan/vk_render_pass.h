#pragma once

#include "gfx/render_pass.h"
#include "gfx/resource/gpu_resource.h"
#include "resources/vk_resource_render_pass.h"
#include "vk_descriptor_pool.h"

namespace gfx::vulkan
{
class RenderPass_VK final : public RenderPass
{
  public:
    RenderPass_VK(const Config& frame_graph_config, bool in_present_pass);
    ~RenderPass_VK() override = default;

    [[nodiscard]] const TGpuHandle<RenderPassResource_VK>& get() const
    {
        return render_pass;
    }

    DescriptorPoolManager& get_descriptor_pool_manager()
    {
        return *descriptor_pool_manager;
    }

  private:
    const std::unique_ptr<DescriptorPoolManager> descriptor_pool_manager;
    TGpuHandle<RenderPassResource_VK>            render_pass;
};
} // namespace gfx::vulkan