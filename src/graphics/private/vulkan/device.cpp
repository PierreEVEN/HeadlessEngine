#include "vulkan/device.h"
#include "vulkan/hardware.h"

#define GLFW_INCLUDE_VULKAN
#include "vulkan/allocator.h"
#include "vulkan/assertion.h"

#include <GLFW/glfw3.h>
#include <set>

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{

static VkDevice logical_device = nullptr;

namespace device
{
void create()
{
    /*
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set                             unique_queue_families = {queue_families.graphic_family.value(), queue_families.present_family.value()};
    float                                queue_priorities      = 1.0f;
    for (uint32_t queueFamily : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queue_priorities;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures device_features{
        .geometryShader    = VK_TRUE,
        .sampleRateShading = VK_TRUE, // Sample Shading
        .fillModeNonSolid  = VK_TRUE, // Wireframe
        .wideLines         = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };

    VkDeviceCreateInfo create_infos = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = NULL,
        .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos       = queueCreateInfos.data(),
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures        = &device_features,
    };

    VK_CHECK(vkCreateDevice(get_physical_device(), &create_infos, get_allocator(), &logical_device), "failed to create device");
*/
}
void destroy()
{
    vkDestroyDevice(logical_device, get_allocator());
}
} // namespace device

VkDevice get_device()
{
    if (!logical_device)
        LOG_FATAL("logical device must be initialized first");

    return logical_device;
}
} // namespace gfx::vulkan