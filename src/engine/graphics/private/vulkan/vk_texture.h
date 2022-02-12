#pragma once

#include "vk_helper.h"
#include "resources/vk_resource_texture.h"

#include "gfx/texture.h"

#include "vulkan/vk_unit.h"
#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class Texture_VK final : public Texture
{
  public:
    Texture_VK(const std::string& name, uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters);
    Texture_VK(const std::string& name, uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters, SwapchainImageResource<VkImage>& existing_images);
    ~Texture_VK() override = default;

    void set_pixels(const std::vector<uint8_t>& data) override;

    [[nodiscard]] const SwapchainImageResource<TGpuHandle<ImageViewResource_VK>>& get_views() const
    {
        return views;
    }

    [[nodiscard]] const SwapchainImageResource<TGpuHandle<ImageResource_VK>>& get_images() const
    {
        return images;
    }

    [[nodiscard]] const VkDescriptorImageInfo& get_descriptor_image_infos()const
    {
        return (*views)->descriptor_infos;
    }

  private:
    SwapchainImageResource<TGpuHandle<ImageResource_VK>>     images;
    SwapchainImageResource<TGpuHandle<ImageViewResource_VK>> views;
};
} // namespace gfx::vulkan
