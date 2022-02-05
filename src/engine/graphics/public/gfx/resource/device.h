#pragma once
#include "gpu_resource.h"
#include "resource_list.h"

#include <unordered_set>

namespace gfx
{
class Device
{
  public:
    // SINGLETON
    static Device&                                               get();
    static void                                                  destroy_device();
    template <typename Device_T, typename... Args_T> static void create_device(Args_T&&... args)
    {
        auto* device = new Device_T(std::forward<Args_T>(args)...);
        device->init();
    }

    void begin_frame(uint8_t frame_index)
    {
        // free resources
        for (const auto& resource : deletion_queues[current_frame_id])
            delete resource;
        deletion_queues[current_frame_id].clear();
        // init new frame
        current_frame_id = frame_index;
    }

    virtual void wait_device() = 0;

    [[nodiscard]] uint8_t get_frame_count() const
    {
        return frame_count;
    }

    [[nodiscard]] uint8_t get_current_frame() const
    {
        return current_frame_id;
    }

  protected:
    Device(uint8_t image_count);
    virtual ~Device();

    virtual void init() = 0;
    void         free_allocations();

  private:
    friend class IGpuHandle;
    friend void destroy_device();

    void delete_resource(IGpuHandle::IResourceReference* resource)
    {
        deletion_queues[current_frame_id].emplace_back(resource);
    }

    std::vector<std::vector<IGpuHandle::IResourceReference*>> deletion_queues;

    uint8_t current_frame_id;
    uint8_t frame_count;
};
} // namespace gfx