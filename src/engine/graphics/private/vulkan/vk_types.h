#pragma once

#include <vulkan/vulkan.h>
#include "gfx/types.h"

namespace gfx::vulkan
{
VkFormat vk_texture_format_to_engine(ETypeFormat format);
ETypeFormat engine_texture_format_from_vk(VkFormat format);
}
