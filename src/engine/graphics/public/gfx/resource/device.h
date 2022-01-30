#pragma once
#include "resource_list.h"

#include <unordered_set>

#define GPU_NULL_HANDLE nullptr;

namespace gfx
{
class Device final
{
  public:
    static Device& get();
    static void    create_device();
    static void    destroy_device();
    ~Device();

    BufferHandle        create_buffer(const CI_Buffer& create_infos);
    CommandBufferHandle create_command_buffer(const CI_CommandBuffer& create_infos);

  private:
    friend class IGpuResource;
    void destroy_resource(ResourceHandle resource_handle);

    std::unordered_set<ResourceHandle> resources;
};
} // namespace gfx