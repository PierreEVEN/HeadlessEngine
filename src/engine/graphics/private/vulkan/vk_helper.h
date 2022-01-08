#pragma once

#include "vk_device.h"

#include <cpputils/logger.hpp>

#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{
template <typename Object_T> void debug_set_object_name([[maybe_unused]] const std::string& object_name, [[maybe_unused]] const Object_T& object)
{
#ifdef ENABLE_VALIDATION_LAYER

    VkDebugReportObjectTypeEXT object_type = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;

    if (typeid(VkInstance) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    else if (typeid(VkPhysicalDevice) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
    else if (typeid(VkDevice) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT;
    else if (typeid(VkQueue) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
    else if (typeid(VkSemaphore) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT;
    else if (typeid(VkCommandBuffer) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
    else if (typeid(VkFence) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT;
    else if (typeid(VkDeviceMemory) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
    else if (typeid(VkBuffer) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
    else if (typeid(VkImage) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
    else if (typeid(VkEvent) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT;
    else if (typeid(VkQueryPool) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT;
    else if (typeid(VkBufferView) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT;
    else if (typeid(VkImageView) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT;
    else if (typeid(VkShaderModule) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
    else if (typeid(VkPipelineCache) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT;
    else if (typeid(VkPipelineLayout) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT;
    else if (typeid(VkRenderPass) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT;
    else if (typeid(VkPipeline) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT;
    else if (typeid(VkDescriptorSetLayout) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT;
    else if (typeid(VkSampler) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT;
    else if (typeid(VkDescriptorPool) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT;
    else if (typeid(VkDescriptorPool) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT;
    else if (typeid(VkFramebuffer) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT;
    else if (typeid(VkCommandPool) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT;
    else if (typeid(VkSurfaceKHR) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT;
    else if (typeid(VkSwapchainKHR) == typeid(Object_T))
        object_type = VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT;
    else
        LOG_FATAL("unhandled debug object type");

    const auto                     pfn_set_object_name = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(get_device(), "vkDebugMarkerSetObjectNameEXT"));
    VkDebugMarkerObjectNameInfoEXT object_name_info    = {
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = object_type,
        .object      = reinterpret_cast<uint64_t>(object),
        .pObjectName = object_name.c_str(),
    };
    pfn_set_object_name(get_device(), &object_name_info);

#endif
}

inline void debug_add_marker([[maybe_unused]] const std::string& marker_name, [[maybe_unused]] VkCommandBuffer command_buffer, [[maybe_unused]] std::array<float, 4> color)
{
#ifdef ENABLE_VALIDATION_LAYER
    // add marker
    const auto                 pfn_debug_marker_begin = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(get_device(), "vkCmdDebugMarkerBeginEXT"));
    VkDebugMarkerMarkerInfoEXT begin_marker           = {
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
        .pNext       = nullptr,
        .pMarkerName = marker_name.c_str(),
    };
    memcpy(begin_marker.color, color.data(), sizeof(float) * 4);
    pfn_debug_marker_begin(command_buffer, &begin_marker);
#endif
}

inline void debug_end_marker([[maybe_unused]] VkCommandBuffer command_buffer)
{
#ifdef ENABLE_VALIDATION_LAYER
    const auto pfn_debug_marker_end = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(get_device(), "vkCmdDebugMarkerEndEXT"));
    pfn_debug_marker_end(command_buffer);
#endif
}

} // namespace gfx::vulkan