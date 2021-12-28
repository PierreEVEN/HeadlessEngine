#pragma once

#include "gfx/texture.h"
#include "vk_mem_alloc.h"
#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{
class VkTexture : public Texture
{
  public:
    VkTexture(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters);
    ~VkTexture();

    void set_pixels(const std::vector<uint8_t>& data) override;

    void update_image_layout(VkCommandBuffer command_buffer, VkImageLayout new_layout);

  private:
    VkImageLayout image_layout;
    VkImage       image;
    VkImageView   view;
    VmaAllocation allocation;
};

} // namespace gfx::vulkan
