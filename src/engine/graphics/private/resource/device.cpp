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


void Device::destroy_resource(ResourceHandle resource_handle)
{
    delete resource_handle;
    resources.erase(resource_handle);
}

Device::~Device()
{
    if (!resources.empty())
        LOG_ERROR("some objects have not been destroyed yet");

    for (const auto& item : resources)
    {
        LOG_WARNING("destroy resource %s", item->name.c_str());
        item->release(0xFF);
        item->destroy();
    }
}
} // namespace gfx