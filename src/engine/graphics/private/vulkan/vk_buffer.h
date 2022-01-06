#pragma once

#include "gfx/buffer.h"

namespace gfx::vulkan
{
class Buffer_VK : public Buffer
{
public:
    Buffer_VK(const std::string& buffer_name, uint32_t size, EBufferUsage usage, EBufferAccess buffer_access = EBufferAccess::DEFAULT) : Buffer(buffer_name, size, usage, buffer_access)

  private:
};


}