#pragma once

#include "render_pass_description.h"

#include <vulkan/vulkan.hpp>

class Swapchain;

class RenderPass
{
  public:
    RenderPass(const RenderPassSettings& in_pass_description);
    virtual ~RenderPass();

    [[nodiscard]] VkRenderPass get_render_pass() const
    {
        return render_pass;
    }

    [[nodiscard]] const RenderPassSettings& get_render_pass_description() const
    {
        return pass_description;
    }

  private:
    void create_render_pass();
    void destroy_render_pass();

    RenderPassSettings pass_description;
    Swapchain*         swapchain   = nullptr;
    VkRenderPass       render_pass = nullptr;
};
