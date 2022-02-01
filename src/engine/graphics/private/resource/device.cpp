#include "gfx/resource/device.h"

#include "gfx/resource/gpu_resource.h"

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
    LOG_WARNING("todo : destroy device more properly");
    device_instance->~Device();
    //delete device_instance;
    //device_instance = nullptr;
}


Device::Device(uint8_t image_count) : current_frame_id(0), frame_count(image_count)
{
    acquired_resources.resize(frame_count, {});
}

void Device::release_frame(uint8_t frame)
{
    for (const auto& resource : acquired_resources[frame])
        resource->release(frame);
    acquired_resources[frame].clear();
}

void Device::set_frame(uint8_t frame_id)
{
    current_frame_id = frame_id;
}

void Device::register_resource(ResourceHandle handle)
{
    resources.insert(handle);
}

void Device::create_device(Device* device)
{
    if (device_instance)
        LOG_FATAL("cannot create device twice");

    device_instance = device;
}

void Device::destroy_resource(ResourceHandle resource_handle)
{
    delete resource_handle;
    resources.erase(resource_handle);
}

Device::~Device()
{
    //@TODO : VkDeviceWaitIdle
    for (uint8_t i = 0; i < frame_count; ++i)
        release_frame(i);

    if (!resources.empty())
        LOG_ERROR("some objects have not been destroyed yet");

    while (!resources.empty())
    {
        const auto& elem = *resources.begin();
        if (!elem)
            continue;

        LOG_WARNING("destroy resource %s", elem->get_name().c_str());
        elem->destroy();
    }
}
} // namespace gfx