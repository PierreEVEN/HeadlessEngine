#pragma once

#include "gfx/render_pass.h"
#include "vulkan/vk_texture.h"

#include <string>
#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class RenderPassResource_VK final
{
  public:
    struct CreateInfos
    {
        const RenderPass::Config& render_pass_config;
        bool                      in_present_pass;
    };

    RenderPassResource_VK(const std::string& name, const CreateInfos& create_infos);
    ~RenderPassResource_VK();

    VkRenderPass render_pass;
    const uint32_t color_attachment_count;
};

class FramebufferResource_VK final
{
  public:
    struct CreateInfos
    {
        uint32_t                                      width;
        uint32_t                                      height;
        TGpuHandle<RenderPassResource_VK>             render_pass;
        std::vector<TGpuHandle<ImageViewResource_VK>> framebuffer_views;
    };

    FramebufferResource_VK(const std::string& name, const CreateInfos& create_infos);
    ~FramebufferResource_VK();

    VkFramebuffer framebuffer = VK_NULL_HANDLE;

  private:
    const TGpuHandle<RenderPassResource_VK>             render_pass;
    const std::vector<TGpuHandle<ImageViewResource_VK>> framebuffer_views;
};
} // namespace gfx::vulkan