#pragma once

#include <vulkan/vulkan.h>

namespace gfx::vulkan
{

namespace device
{
void create();
void destroy();

} // namespace device
VkDevice get_device();

} // namespace gfx::vulkan