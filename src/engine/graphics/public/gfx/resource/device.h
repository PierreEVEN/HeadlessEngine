#pragma once
#include "resource_list.h"

#include <unordered_set>

namespace gfx
{
class Device
{
  public:
    static Device& get();

    template <typename Device_T, typename... Args_T> static void create_device(Args_T&&... args)
    {
        create_device(new Device_T(std::forward<Args_T>(args)...));
    }

    static void destroy_device();
    Device(uint8_t image_count);
    virtual ~Device();

    virtual BufferHandle        create_buffer(const std::string& name, const CI_Buffer& create_infos)                = 0;
    virtual CommandBufferHandle create_command_buffer(const std::string& name, const CI_CommandBuffer& create_infos) = 0;

    void acquire_resource(ResourceHandle resource)
    {
        acquired_resources[current_frame_id].emplace_back(resource);
    }

    void release_frame(uint8_t frame);

    void set_frame(uint8_t frame_id);

    [[nodiscard]] uint8_t get_current_frame() const
    {
        return current_frame_id;
    }
    [[nodiscard]] uint8_t get_frame_count() const
    {
        return frame_count;
    }

    void register_resource(ResourceHandle handle);

  private:
    static void create_device(Device* device);

    friend class IGpuResource;
    void destroy_resource(ResourceHandle resource_handle);

    std::unordered_set<ResourceHandle>       resources;
    std::vector<std::vector<ResourceHandle>> acquired_resources;

    uint8_t current_frame_id;
    uint8_t frame_count;
};
} // namespace gfx