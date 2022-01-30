#pragma once

#include "gfx/resource_handle.h"
namespace gfx
{
void IGpuResource::destroy()
{
    should_destroy = true;
    if (!used)
    {
        Device::get().destroy_resource(this);
    }
}

BufferHandle Device::create_buffer(const CI_Buffer& create_infos)
{
    ResourceHandle handle = GPU_NULL_HANDLE;
    if (GFX_USE_VULKAN)
    {
        handle = new TGpuResource<BufferVK_H>("test buffer");
    }
    else
    {
        handle = GPU_NULL_HANDLE;
    }

    if (handle)
        resources.insert(handle);

    return handle;
}

CommandBufferHandle Device::create_command_buffer(const CI_CommandBuffer& create_infos)
{
#if GFX_USE_VULKAN
    const auto handle = new TGpuResource<CommandBuffer_VK>("test command buffer");
    resources.insert(handle);
    return handle;
#endif
}

void Device::destroy_resource(ResourceHandle resource_handle)
{
    delete resource_handle;
    resources.erase(resource_handle);
}


Device::~Device()
{
    if (!resources.empty())
        LOG_ERROR("some objects have not been destroyed yet");

    for (const auto& item : resources)
        handle_cast<BufferVK_H>(item)->destroy();
}

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

    acquire_resource(handle);
#if GFX_USE_VULKAN
    BufferVK_H& cmd = handle_cast<BufferVK_H>(handle)->use();
    (void)cmd;
#endif
}
} // namespace gfx
