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


Device::Device(uint8_t image_count) : current_frame_id(0), frame_count(image_count)
{
    if (device_instance)
        LOG_FATAL("device have already been created");
    device_instance = this;

    deletion_queues.resize(frame_count, {});
}

void Device::free_allocations()
{
    wait_device();

    for (auto& queue : deletion_queues)
    {
        for (const auto& resource : queue)
            delete resource;
        queue.clear();
    }
}

Device::~Device()
{
    device_instance = nullptr;
}
} // namespace gfx