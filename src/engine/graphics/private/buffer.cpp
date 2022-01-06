

#include "gfx/buffer.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/vk_buffer.h"
#endif

namespace gfx
{
Buffer::Buffer(const std::string& in_buffer_name, uint32_t buffer_stride, uint32_t elements, EBufferUsage buffer_usage, EBufferAccess in_buffer_access)
    : stride(buffer_stride), element_count(elements), buffer_name(in_buffer_name), buffer_access(in_buffer_access), usage(buffer_usage)
{
}

std::shared_ptr<Buffer> Buffer::create(const std::string& buffer_name, uint32_t element_count, uint32_t stride, EBufferUsage usage, EBufferAccess buffer_access)
{
#if GFX_USE_VULKAN
    return std::make_shared<vulkan::Buffer_VK>(buffer_name, stride, element_count, usage, buffer_access);
#else
    return nullptr;
#endif
}

std::shared_ptr<Buffer> Buffer::create(const std::string& buffer_name, uint32_t buffer_size, EBufferUsage usage, EBufferAccess buffer_access)
{
    return create(buffer_name, buffer_size, 1, usage, buffer_access);
}

Buffer::~Buffer()
{
}

} // namespace gfx