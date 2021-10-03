#pragma once

#include "assets/asset_texture.h"
#include "rendering/renderer/swapchain.h"

#include <vulkan/vulkan.hpp>

class Swapchain;
class RenderPass;

class AFramebufferImage final : public ATexture
{
  public:
    AFramebufferImage(class Swapchain* source_swapchain);
    AFramebufferImage(const VkExtent2D& image_extent, VkSampleCountFlagBits sample_count, VkFormat image_format, bool is_depth_stencil, int image_count = 1, bool b_use_sampler = false);
    ~AFramebufferImage() override;

    [[nodiscard]] VkImage get_image(uint32_t image_index = 0) const override
    {
        return images.empty() ? VK_NULL_HANDLE : images[images.size() == 1 ? 0 : image_index];
    }
    [[nodiscard]] VkImageView get_view(uint32_t image_index = 0) const override
    {
        return views.empty() ? VK_NULL_HANDLE : views[views.size() == 1 ? 0 : image_index];
    }

    [[nodiscard]] VkImageLayout get_image_layout(uint32_t image_index = 0) const override
    {
        return image_layout;
        ;
    }

    [[nodiscard]] VkSampler get_sampler(uint32_t image_index = 0) const override
    {
        return sampler;
    }

  private:
    void create_sampler();

    std::vector<VkImage>        images       = {};
    std::vector<VkDeviceMemory> memories     = {};
    std::vector<VkImageView>    views        = {};
    VkImageLayout               image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampler                   sampler      = VK_NULL_HANDLE;
};

class Framebuffer
{
  public:
    Framebuffer(RenderPass* used_render_pass, VkExtent2D in_buffer_size, Swapchain* current_swapchain = nullptr);
    virtual ~Framebuffer();

    [[nodiscard]] VkFramebuffer& get(size_t image)
    {
        return framebuffers[image];
    }

  private:
    struct BufferImageRef
    {
        bool                        per_swapchain_image = false;
        std::vector<VkImage>        images              = {};
        std::vector<VkDeviceMemory> memories            = {};
        std::vector<VkImageView>    views               = {};
    };

    void create_framebuffer_images();
    void create_framebuffer();

    void destroy_framebuffer_images();
    void destroy_framebuffer();

    RenderPass* render_pass = nullptr;
    Swapchain*  swapchain   = nullptr;

    std::vector<TAssetPtr<AFramebufferImage>> framebuffer_images;

    std::vector<VkFramebuffer> framebuffers;

    VkExtent2D buffer_size;
};
