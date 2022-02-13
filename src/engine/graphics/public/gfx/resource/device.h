#pragma once
#include "gpu_resource.h"
#include "resource_list.h"

#include <unordered_set>

namespace gfx
{
class Device
{
  public:
    struct CI_Device
    {
        bool    bindless_descriptors = true;
        uint8_t swapchain_images     = 3;
    };

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
        // update current frame
        current_frame_id = frame_index;

        // free resources (don't use for loop because deletion queue size can be increased)
        for (size_t i = 0; i < deletion_queues[current_frame_id].size(); ++i)
        {
            delete deletion_queues[current_frame_id][i];
        }

        deletion_queues[current_frame_id].clear();
    }

    virtual void wait_device() = 0;

    [[nodiscard]] uint8_t get_frame_count() const
    {
        return parameters.swapchain_images;
    }

    [[nodiscard]] uint8_t get_current_frame() const
    {
        return current_frame_id;
    }

    [[nodiscard]] bool use_bindless_descriptors() const
    {
        return parameters.bindless_descriptors;
    }

    [[nodiscard]] virtual bool has_bindless_support() const
    {
        return false;
    }

  protected:
    Device(const CI_Device& image_count);
    virtual ~Device();

    virtual void init() = 0;
    void         free_allocations();

  private:
    friend class IGpuHandle;
    friend void destroy_device();

    CI_Device parameters;

    void delete_resource(IGpuHandle::IResourceReference* resource)
    {
        deletion_queues[current_frame_id].emplace_back(resource);
    }

    std::vector<std::vector<IGpuHandle::IResourceReference*>> deletion_queues;

    uint8_t current_frame_id;
};
} // namespace gfx