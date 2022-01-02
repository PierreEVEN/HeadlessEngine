#pragma once

#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{
namespace instance
{
void create();
void destroy();
}

const VkInstance& get_instance();

} // namespace gfx::vulkan