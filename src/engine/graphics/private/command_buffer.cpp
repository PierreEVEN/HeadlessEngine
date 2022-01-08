#include "gfx/command_buffer.h"

#if GFX_USE_VULKAN
#include "vulkan/vk_command_buffer.h"
#endif

namespace gfx
{
CommandBuffer* CommandBuffer::create(const std::string& name)
{
#if GFX_USE_VULKAN
    return new vulkan::CommandBuffer_VK(name);
#else
    return nullptr;
#endif
}
} // namespace gfx
