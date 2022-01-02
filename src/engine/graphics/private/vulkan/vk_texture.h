#pragma once

#include "gfx/texture.h"
#include "vk_mem_alloc.h"

#include "types/magic_enum.h"
#include "unit.h"
#include <cpputils/logger.hpp>
#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{
class VkTexture : public Texture
{
  public:
    VkTexture(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters);
    VkTexture(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters, SwapchainImageResource<VkImage>& existing_images);
    ~VkTexture();

    void set_pixels(const std::vector<uint8_t>& data) override;

    void update_image_layout(VkCommandBuffer command_buffer, VkImageLayout new_layout);

    [[nodiscard]] const SwapchainImageResource<VkImageView>& get_view() const
    {
        return view;
    }

    static VkFormat vk_texture_format_to_engine(EImageFormat format)
    {
        switch (format)
        {
        case EImageFormat::R_UNORM_8:
            return VK_FORMAT_R8_UNORM;
        case EImageFormat::RG_UNORM_8:
            return VK_FORMAT_R8G8_UNORM;
        case EImageFormat::RGB_UNORM_8:
            return VK_FORMAT_R8G8B8_UNORM;
        case EImageFormat::RGBA_UNORM_8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case EImageFormat::R_SRGB_8:
            return VK_FORMAT_R8_SRGB;
        case EImageFormat::RG_SRGB_8:
            return VK_FORMAT_R8G8_SRGB;
        case EImageFormat::RGB_SRGB_8:
            return VK_FORMAT_R8G8B8_SRGB;
        case EImageFormat::RGBA_SRGB_8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case EImageFormat::R_FLOAT_16:
            return VK_FORMAT_R16_SFLOAT;
        case EImageFormat::RG_FLOAT_16:
            return VK_FORMAT_R16G16_SFLOAT;
        case EImageFormat::RGB_FLOAT_16:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case EImageFormat::RGBA_FLOAT_16:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case EImageFormat::R_FLOAT_32:
            return VK_FORMAT_R32_SFLOAT;
        case EImageFormat::RG_FLOAT_32:
            return VK_FORMAT_R32G32_SFLOAT;
        case EImageFormat::RGB_FLOAT_32:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case EImageFormat::RGBA_FLOAT_32:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case EImageFormat::BGRA_UNORM_8:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case EImageFormat::DEPTH_32_FLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case EImageFormat::DEPTH_32_STENCIL_8_FLOAT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case EImageFormat::DEPTH_24_STENCIL_8_UNORM:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        }
        LOG_FATAL("images format %d : %s is not supported", format, magic_enum::enum_name(format).data());
        return VK_FORMAT_UNDEFINED;
    }

    static EImageFormat engine_texture_format_from_vk(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_R8_UNORM:
            return EImageFormat::R_UNORM_8;
        case VK_FORMAT_R8G8_UNORM:
            return EImageFormat::RG_UNORM_8;
        case VK_FORMAT_R8G8B8_UNORM:
            return EImageFormat::RGB_UNORM_8;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return EImageFormat::RGBA_UNORM_8;
        case VK_FORMAT_R8_SRGB:
            return EImageFormat::R_SRGB_8;
        case VK_FORMAT_R8G8_SRGB:
            return EImageFormat::RG_SRGB_8;
        case VK_FORMAT_R8G8B8_SRGB:
            return EImageFormat::RGB_SRGB_8;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return EImageFormat::RGBA_SRGB_8;
        case VK_FORMAT_R16_SFLOAT:
            return EImageFormat::R_FLOAT_16;
        case VK_FORMAT_R16G16_SFLOAT:
            return EImageFormat::RG_FLOAT_16;
        case VK_FORMAT_R16G16B16_SFLOAT:
            return EImageFormat::RGB_FLOAT_16;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return EImageFormat::RGBA_FLOAT_16;
        case VK_FORMAT_R32_SFLOAT:
            return EImageFormat::R_FLOAT_32;
        case VK_FORMAT_R32G32_SFLOAT:
            return EImageFormat::RG_FLOAT_32;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return EImageFormat::RGB_FLOAT_32;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return EImageFormat::RGBA_FLOAT_32;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return EImageFormat::BGRA_UNORM_8;
        default:
            LOG_FATAL("unsupported format : %d", format);
        }
    }

  private:

    void create_views();

    const bool                            use_external_images;
    SwapchainImageResource<VkImageLayout> image_layout;
    SwapchainImageResource<VkImage>       images;
    SwapchainImageResource<VmaAllocation> allocation;
    SwapchainImageResource<VkImageView>   view;
};
} // namespace gfx::vulkan
