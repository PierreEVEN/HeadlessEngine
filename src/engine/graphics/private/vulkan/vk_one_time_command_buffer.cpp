
#include "vulkan/vk_one_time_command_buffer.h"

#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_device.h"
#include "vk_physical_device.h"

namespace gfx::vulkan
{

OneTimeCommandBuffer::OneTimeCommandBuffer(EQueueFamilyType queue_family) : queue(queue_family)
{
    const VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = command_pool::get(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkAllocateCommandBuffers(get_device(), &alloc_info, &command_buffer);

    const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(command_buffer, &beginInfo);
}

OneTimeCommandBuffer::~OneTimeCommandBuffer()
{
    vkEndCommandBuffer(command_buffer);

    const VkSubmitInfo submit_infos = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &command_buffer,
    };

    const auto fence = get_physical_device<PhysicalDevice_VK>()->submit_queue(queue, submit_infos);
    vkWaitForFences(get_device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkFreeCommandBuffers(get_device(), command_pool::get(), 1, &command_buffer);
}
} // namespace gfx::vulkan