#include "gfx/resource/device.h"

#include <cpputils/logger.hpp>

namespace gfx
{

static Device* device_instance = nullptr;

Device& Device::get()
{
    if (!device_instance)
        LOG_FATAL("device have not been created yet");

    return *device_instance;
}

void Device::destroy_device()
{
    delete device_instance;
}


Device::Device(const CI_Device& create_infos) : parameters(create_infos), current_frame_id(0)
{
    if (device_instance)
        LOG_FATAL("device have already been created");
    device_instance = this;

    deletion_queues.resize(parameters.swapchain_images, {});
}

void Device::free_allocations()
{
    wait_device();

    for (auto& queue : deletion_queues)
    {
        while (!queue.empty())
        {
            const auto* back = queue.back();
            queue.pop_back();
            delete back;
        }
        queue.clear();
    }
}

Device::~Device()
{
    device_instance = nullptr;
}
} // namespace gfx