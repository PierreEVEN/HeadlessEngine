
#include "vulkan/vk_types.h"

namespace gfx::vulkan
{
VkFormat vk_texture_format_to_engine(ETypeFormat format)
{
    return static_cast<VkFormat>(format);
}

ETypeFormat engine_texture_format_from_vk(VkFormat format)
{
    return static_cast<ETypeFormat>(format);
}

} // namespace gfx::vulkan