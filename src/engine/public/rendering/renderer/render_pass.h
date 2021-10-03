#pragma once

#include <vulkan/vulkan.hpp>

class Swapchain;

class RenderPass
{
public:

    RenderPass(Swapchain* target_swapchain);
  virtual ~RenderPass();

    [[nodiscard]] VkRenderPass get_render_pass() const
  {
      return render_pass;
  }

private:

    void create_or_recreate_render_pass();
    void destroy_render_pass();

    Swapchain*   swapchain = nullptr;
    VkRenderPass render_pass = nullptr;
};
