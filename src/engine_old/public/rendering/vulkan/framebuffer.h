#pragma once

#include "assets/asset_texture.h"
#include "rendering/renderer/swapchain.h"

#include <vulkan/vulkan.hpp>

class Swapchain;
class RenderPass;

struct FramebufferImageInfo
{
    VkExtent2D            image_extent     = {0, 0};
    VkSampleCountFlagBits sample_count     = VK_SAMPLE_COUNT_1_BIT;
    VkFormat              image_format     = VK_FORMAT_UNDEFINED;
    bool                  is_depth_stencil = true;
    int                   image_count      = 1;
    bool                  b_use_sampler    = false;
};

class AFramebufferImage final : public ATexture
{
  public:
    AFramebufferImage(class Swapchain* in_source_swapchain);
    AFramebufferImage(const FramebufferImageInfo& in_framebuffer_image_info);
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
    }

    [[nodiscard]] VkSampler get_sampler(uint32_t image_index = 0) const override
    {
        return sampler;
    }

    void resize_buffer(VkExtent2D in_buffer_size);

  private:
    void create_or_recreate();
    void create_sampler();
    void delete_resources();

    Swapchain*                  source_swapchain = nullptr;
    FramebufferImageInfo        framebuffer_image_info = {};
    std::vector<VkImage>        images           = {};
    std::vector<VkDeviceMemory> memories         = {};
    std::vector<VkImageView>    views            = {};
    VkImageLayout               image_layout     = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampler                   sampler          = VK_NULL_HANDLE;
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

    void resize_framebuffer(const VkExtent2D& new_extent);

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
