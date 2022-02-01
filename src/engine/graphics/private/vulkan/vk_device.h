#pragma once

#include "gfx/resource/device.h"
#include "vk_buffer.h"
#include "vk_command_buffer.h"
#include "gfx/resource/gpu_resource.h"

#include <vulkan/vulkan.h>

namespace gfx::vulkan
{

namespace device
{
void create();
void destroy();

} // namespace device
VkDevice get_device();

class Device_VK : public Device
{
  public:

      Device_VK(uint8_t image_count)
        : Device(image_count) {}

    BufferHandle create_buffer(const std::string& name, const CI_Buffer& create_infos) override
    {
        return new TGpuResource<BufferResource_VK>(name, create_infos);
    }

    CommandBufferHandle create_command_buffer(const std::string& name, const CI_CommandBuffer& create_infos) override
    {
        return new TGpuResource<CommandBufferResource_VK>(name, create_infos);
    }
};

} // namespace gfx::vulkan