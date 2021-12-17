#include "gfx/physical_device.h"

#include "gfx/instance.h"

#if GFX_USE_VULKAN
#include "vulkan/vk_physical_device.h"
#include "vulkan/instance.h"
#endif


#include <memory>

namespace gfx
{
static std::vector<std::unique_ptr<PhysicalDevice>> devices;

void fetch_physical_devices()
{
    devices.clear();

#if GFX_USE_VULKAN
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkan::get_instance(), &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> found_devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkan::get_instance(), &deviceCount, found_devices.data());

    for (const auto& device : found_devices)
        devices.emplace_back(std::make_unique<vulkan::VulkanPhysicalDevice>(device));
#endif
}

PhysicalDevice* find_best_physical_device(const std::vector<PhysicalDevice*>& candidates)
{
    if (candidates.empty())
        return nullptr;

    for (const auto& device : candidates)
    {
        if (device->get_device_type() == EPhysicalDeviceType::DEDICATED_GPU)
            return device;
    }

    for (const auto& device : candidates)
    {
        if (device->get_device_type() == EPhysicalDeviceType::INTEGRATED_GPU)
            return device;
    }

    return candidates[0];
}

std::vector<PhysicalDevice*> get_physical_devices()
{
    std::vector<PhysicalDevice*> out_devices(devices.size());
    for (int i = 0; i < devices.size(); ++i)
        out_devices[i] = devices[i].get();

    return out_devices;
}

static PhysicalDevice* physical_device;

void select_physical_device(PhysicalDevice* device)
{
    physical_device = device;
}

PhysicalDevice* get_physical_device_internal()
{
    return physical_device;
}
} // namespace gfx
