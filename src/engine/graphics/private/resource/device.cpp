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

void Device::create_device()
{
    if (device_instance)
        LOG_FATAL("cannot create device twice");

    device_instance = new Device();
}

void Device::destroy_device()
{
    delete device_instance;
    device_instance = nullptr;
}
} // namespace gfx