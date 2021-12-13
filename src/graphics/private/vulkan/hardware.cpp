
#include "device.h"
#include "instance.h"
#include <cpputils/logger.hpp>

namespace gfx::vulkan
{
static VkPhysicalDevice physical_device = VK_NULL_HANDLE;

namespace hardware
{
void select_hardware()
{
    // Get devices
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(get_instance(), &device_count, nullptr);
    if (device_count == 0)
        LOG_FATAL("No graphical device found.");
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(get_instance(), &device_count, devices.data());

    // Enumerate devices
    std::string PhysLog = "Found " + std::to_string(device_count) + " graphical devices : \n";
    for (const VkPhysicalDevice& device : devices)
    {
        VkPhysicalDeviceProperties pProperties;
        vkGetPhysicalDeviceProperties(device, &pProperties);
        PhysLog += "\t-" + std::string(pProperties.deviceName) + " (driver version : " + std::to_string(pProperties.driverVersion) + ")\n";
    }
    LOG_INFO("[ Core] %s", PhysLog.c_str());

    // Pick desired device
    for (const auto& device : devices)
    {
        // @TODO : test if device is suitable (check extensions ...)
        physical_device = device;
        return;
    }
}
}

VkPhysicalDevice get_physical_device()
{
    if (physical_device == VK_NULL_HANDLE)
        LOG_FATAL("physical device must be initialized first");
    return physical_device;
}
} // namespace gfx::vulkan
