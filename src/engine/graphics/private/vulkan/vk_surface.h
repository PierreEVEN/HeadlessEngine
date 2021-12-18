#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include "gfx/surface.h"

namespace gfx::vulkan
{
class Surface_VK : public Surface
{
  public:
    Surface_VK(application::window::Window* container);
    virtual ~Surface_VK();

    void submit_command_buffer(const CommandBuffer* command_buffer) override
    {
        (void)command_buffer;
    }

  private:
    VkSurfaceKHR surface;
};
} // namespace gfx::vulkan