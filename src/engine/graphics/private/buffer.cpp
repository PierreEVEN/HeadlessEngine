

#include "gfx/buffer.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/vk_buffer.h"
#endif

namespace gfx
{
Buffer::Buffer(const std::string& in_buffer_name, uint32_t buffer_stride, uint32_t elements, EBufferUsage buffer_usage, EBufferAccess in_buffer_access, EBufferType buffer_type)
    : element_stride(buffer_stride), element_count(elements), buffer_name(in_buffer_name), buffer_access(in_buffer_access), usage(buffer_usage), type(buffer_type)
{
    if (element_count <= 0)
    {
        LOG_ERROR("cannot create an empty buffer");
        element_count = 1;
    }
    if (element_stride <= 0)
    {
        LOG_ERROR("buffer element_stride cannot be zero");
        element_stride = 1;
    }
}

std::shared_ptr<Buffer> Buffer::create(const std::string& buffer_name, uint32_t element_count, uint32_t stride, EBufferUsage usage, EBufferAccess buffer_access, EBufferType buffer_type)
{
#if GFX_USE_VULKAN
    return std::make_shared<vulkan::Buffer_VK>(buffer_name, stride, element_count, usage, buffer_access, buffer_type);
#else
    static_assert(false, "backend not supported");
#endif
}

std::shared_ptr<Buffer> Buffer::create(const std::string& buffer_name, uint32_t buffer_size, EBufferUsage usage, EBufferAccess buffer_access, EBufferType buffer_type)
{
    return create(buffer_name, buffer_size, 1, usage, buffer_access, buffer_type);
}

Buffer::~Buffer()
{
}

} // namespace gfx