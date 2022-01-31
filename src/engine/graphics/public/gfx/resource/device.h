#pragma once
#include "gpu_resource.h"
#include "resource_list.h"

#include <unordered_set>

#define GPU_NULL_HANDLE nullptr;

namespace gfx
{
class Device
{
  public:
    static Device& get();
    static void    create_device();
    static void    destroy_device();
    virtual ~Device();

    virtual BufferHandle        create_buffer(const CI_Buffer& create_infos)                = 0;
    virtual CommandBufferHandle create_command_buffer(const CI_CommandBuffer& create_infos) = 0;

    void acquire_resource(ResourceHandle resource)
    {
        acquired_resources[current_frame_id].emplace_back(resource);
    }

    void release_frame(uint8_t frame)
    {
        for (auto& resource : acquired_resources)
            resource[frame]->release(frame);
    }

    uint8_t get_frame_id()
    {
        return current_frame_id;
    }

  private:
    friend class IGpuResource;
    void destroy_resource(ResourceHandle resource_handle);

    std::unordered_set<ResourceHandle>       resources;
    std::vector<std::vector<ResourceHandle>> acquired_resources;

    uint8_t current_frame_id = 0;
};
} // namespace gfx