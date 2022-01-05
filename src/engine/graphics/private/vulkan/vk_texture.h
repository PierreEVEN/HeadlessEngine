#pragma once

#include "gfx/texture.h"
#include "vk_mem_alloc.h"

#include "types/magic_enum.h"
#include "vulkan/vk_unit.h"
#include <cpputils/logger.hpp>
#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{
class Texture_VK : public Texture
{
public:
    Texture_VK(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters);
    Texture_VK(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters, SwapchainImageResource<VkImage>& existing_images);
    virtual ~Texture_VK();

    void set_pixels(const std::vector<uint8_t>& data) override;

    void update_image_layout(VkCommandBuffer command_buffer, VkImageLayout new_layout);

    [[nodiscard]] const SwapchainImageResource<VkImageView>& get_view() const
    {
        return views;
    }

    static VkFormat vk_texture_format_to_engine(ETypeFormat format)
    {
        return static_cast<VkFormat>(format);
    }

    static ETypeFormat engine_texture_format_from_vk(VkFormat format)
    {
        return static_cast<ETypeFormat>(format);
    }

private:
    void create_views();

    const bool                            use_external_images;
    SwapchainImageResource<VkImageLayout> image_layout;
    SwapchainImageResource<VkImage>       images;
    SwapchainImageResource<VmaAllocation> allocation;
    SwapchainImageResource<VkImageView>   views;
};
} // namespace gfx::vulkan
