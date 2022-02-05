#pragma once

#include "gfx/texture.h"
#include "vk_mem_alloc.h"

#include "vulkan/vk_unit.h"
#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class ImageResource_VK final
{
  public:
    // Texture
    struct CI_Texture
    {
        uint32_t                width;
        uint32_t                height;
        uint32_t                depth;
        const TextureParameter texture_parameters;
    };

    ImageResource_VK(const std::string& name, const CI_Texture& create_infos);
    ImageResource_VK(const std::string& name, const CI_Texture& create_infos, const VkImage& existing_image_handle);
    ~ImageResource_VK();

    void set_image_layout(VkCommandBuffer command_buffer, VkImageLayout new_layout);

    void set_pixels(const std::vector<uint8_t>& data);

    size_t get_data_size();

    VkImage image = VK_NULL_HANDLE;

  private:
    VkImageLayout image_layout;
    VmaAllocation memory;
    CI_Texture    image_infos;
};

class ImageViewResource_VK final
{
  public:
    // Texture view
    struct CI_TextureView
    {
        const TextureParameter      texture_parameters;
        TGpuHandle<ImageResource_VK> used_image;
    };

    ImageViewResource_VK(const std::string& name, const CI_TextureView& create_infos);
    ~ImageViewResource_VK();

    VkDescriptorImageInfo descriptor_infos;
    VkImageView           view;
};

class Texture_VK final : public Texture
{
  public:
    Texture_VK(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters);
    Texture_VK(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters, SwapchainImageResource<VkImage>& existing_images);
    ~Texture_VK() override = default;

    void set_pixels(const std::vector<uint8_t>& data) override;

    [[nodiscard]] const SwapchainImageResource<TGpuHandle<ImageViewResource_VK>>& get_view() const
    {
        return views;
    }

    [[nodiscard]] const TGpuHandle<ImageResource_VK>& get_current_image() const
    {
        return *images;
    }

    const VkDescriptorImageInfo& get_descriptor_image_infos()const
    {
        return (*views)->descriptor_infos;
    }

  private:
    SwapchainImageResource<TGpuHandle<ImageResource_VK>>     images;
    SwapchainImageResource<TGpuHandle<ImageViewResource_VK>> views;
};
} // namespace gfx::vulkan
