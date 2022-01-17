#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"

#include <optional>
#include <string>
#include <vector>

#include <cpputils/simplemacros.hpp>

#if CXX_MSVC
#define VK_ENSURE(condition, ...)                              \
    if ((condition) != VK_SUCCESS)                             \
    {                                                          \
        LOG_FATAL("VK_ERROR %d : %s", condition, __VA_ARGS__); \
    }
#define VK_CHECK(object, ...)                                           \
    if ((object) == VK_NULL_HANDLE)                                     \
    {                                                                   \
        LOG_FATAL("VK_ERROR_NULL_HANDLE %d : %s", object, __VA_ARGS__); \
    }
#else
#define VK_ENSURE(condition, ...)                                \
    if ((condition) != VK_SUCCESS)                               \
    {                                                            \
        LOG_FATAL("VK_ERROR %d : %s", condition, ##__VA_ARGS__); \
    }
#define VK_CHECK(object, ...)                                             \
    if ((object) == VK_NULL_HANDLE)                                       \
    {                                                                     \
        LOG_FATAL("VK_ERROR_NULL_HANDLE %d : %s", object, ##__VA_ARGS__); \
    }
#endif

namespace vulkan_utils
{

class SwapchainSupportDetails
{
  public:
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   present_modes;
};

extern VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_infos;

std::vector<const char*> get_required_extensions();
SwapchainSupportDetails  get_swapchain_support_details(VkSurfaceKHR surface, VkPhysicalDevice device);
VkSampleCountFlagBits    get_max_usable_sample_count();
VkFormat                 find_texture_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physical_device);
VkFormat                 get_depth_format();
VkExtent2D               choose_swapchain_extend(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& initial_extend);
uint32_t                 find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
VkCommandBuffer          begin_single_time_commands();
void                     end_single_time_commands(VkCommandBuffer commandBuffer);
void                     create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void                     copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void                     create_vma_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo& allocInfos);
} // namespace vulkan_utils
