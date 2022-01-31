#pragma once

#include "gfx/resource_handle.h"
namespace gfx
{
void CommandBuffer_VK::begin()
{
}

void CommandBuffer_VK::release()
{
    for (auto resource : acquired_resources)
    {
        resource->destroy();
    }
}

void CommandBuffer_VK::bind_buffer(BufferHandle handle)
{
    BufferVK_H* cmd = use_resource<BufferVK_H>(handle);
    (void)cmd;
}
} // namespace gfx
