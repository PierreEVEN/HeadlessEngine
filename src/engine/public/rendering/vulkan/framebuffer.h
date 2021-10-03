#pragma once

class Swapchain;
class RenderPass;

#include "rendering/vulkan/utils.h"

class Framebuffer
{
  public:
    Framebuffer(RenderPass* used_render_pass, Swapchain* current_swapchain);
    virtual ~Framebuffer();

    [[nodiscard]] VkFramebuffer& get(size_t image)
    {
        return framebuffers[image];
    }

  private:
    void create_framebuffer_images();
    void create_framebuffer();

    void destroy_framebuffer_images();
    void destroy_framebuffer();

    RenderPass* render_pass = nullptr;
    Swapchain* swapchain   = nullptr;

    VkImage color_target = VK_NULL_HANDLE;
    VkImage depth_target = VK_NULL_HANDLE;

    VkDeviceMemory color_memory = VK_NULL_HANDLE;
    VkDeviceMemory depth_memory = VK_NULL_HANDLE;

    VkImageView color_view = VK_NULL_HANDLE;
    VkImageView depth_view = VK_NULL_HANDLE;

    std::vector<VkImage>     swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    std::vector<VkFramebuffer> framebuffers;

    VkExtent2D buffer_size;
};
