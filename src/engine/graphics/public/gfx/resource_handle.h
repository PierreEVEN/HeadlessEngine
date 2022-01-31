#pragma once

#include "resource/device.h"
#include "resource/gpu_resource.h"
#include <string>
#include <vector>

namespace gfx
{
/**
 * \brief ///////// ENTER VK API
 */

class BufferVK_H
{
public:
    BufferVK_H(const std::string&)
    {
    }
};

class CommandBuffer_VK
{
  public:
    CommandBuffer_VK(const std::string&) {}
    void begin();
    void release();

    void bind_buffer(BufferHandle handle);

  private:
    void acquire_resource(ResourceHandle handle)
    {
        acquired_resources.emplace_back(handle);
    }

    std::vector<IGpuResource*> acquired_resources;
};

/**
 * \brief ///////// EXIT VK API
 */

inline void renderer_test_vk()
{
    Device device;

    // Init
    const BufferHandle        buff = device.create_buffer({});
    const CommandBufferHandle cmd  = device.create_command_buffer({});

    /**
     * \brief /////// ENTER VK API
     */

    /**
     * \brief ///////// EXIT VK API
     */
}

} // namespace gfx
