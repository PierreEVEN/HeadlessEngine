
#include "gfx/texture.h"

#include "gfx/buffer.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/vk_texture.h"
#endif
namespace gfx
{
Texture::Texture(uint32_t pixel_width, uint32_t pixel_height, uint32_t pixel_depth, const TextureParameter& parameters) : image_parameters(parameters)
{
    if (parameters.format == EImageFormat::UNDEFINED)
        LOG_FATAL("buffer format is undefined");

    switch (image_parameters.image_type)
    {
    case EImageType::Texture_1D:
        width = pixel_width;
        height = 1;
        depth  = 1;
        break;
    case EImageType::Texture_1D_Array:
        width  = pixel_width;
        height = 1;
        depth  = pixel_depth;
        break;
    case EImageType::Texture_2D:
        width  = pixel_width;
        height = pixel_height;
        depth  = 1;
        break;
    case EImageType::Texture_2D_Array:
        width  = pixel_width;
        height = pixel_height;
        depth  = pixel_depth;
        break;
    case EImageType::Texture_3D:
        width  = pixel_width;
        height = pixel_height;
        depth  = pixel_depth;
        break;
    case EImageType::Cubemap:
        width  = pixel_width;
        height = pixel_height;
        depth  = 6;
        break;
    }

    if (!image_parameters.mip_level)
        image_parameters.mip_level = std::optional<uint32_t>(std::floor(log2(std::max(std::max(width, height), depth))) + 1);

}

std::shared_ptr<Texture> Texture::create(const uint32_t width, uint32_t height, const uint32_t depth, const TextureParameter& parameters)
{
#if GFX_USE_VULKAN
    return std::make_shared<vulkan::VkTexture>(width, height, depth, parameters);
#else
    return nullptr;
#endif
}


uint8_t Texture::get_channel_count()
{
    if ((static_cast<int>(image_parameters.format) & static_cast<int>(EImageFormat::CHANNEL_RGBA)) == static_cast<int>(EImageFormat::CHANNEL_RGBA))
        return 4;
    if ((static_cast<int>(image_parameters.format) & static_cast<int>(EImageFormat::CHANNEL_RGB)) == static_cast<int>(EImageFormat::CHANNEL_RGB))
        return 3;
    if ((static_cast<int>(image_parameters.format) & static_cast<int>(EImageFormat::CHANNEL_RG)) == static_cast<int>(EImageFormat::CHANNEL_RG))
        return 2;
    if ((static_cast<int>(image_parameters.format) & static_cast<int>(EImageFormat::CHANNEL_R)) == static_cast<int>(EImageFormat::CHANNEL_R))
        return 1;
    LOG_ERROR("cannot deduce channel count for current texture");
    return 0;
}

uint8_t Texture::get_byte_per_pixel()
{
    if (static_cast<int>(image_parameters.format) & static_cast<int>(EImageFormat::PRECISION_8))
        return 1;
    if (static_cast<int>(image_parameters.format) & static_cast<int>(EImageFormat::PRECISION_16))
        return 2;
    if (static_cast<int>(image_parameters.format) & static_cast<int>(EImageFormat::PRECISION_32))
        return 3;
    if (static_cast<int>(image_parameters.format) & static_cast<int>(EImageFormat::PRECISION_64))
        return 4;
    LOG_ERROR("cannot deduce byte per pixel count for current texture");
    return 0;
}

uint32_t Texture::get_data_size()
{
    return width * height * depth * get_channel_count() * get_byte_per_pixel();
}
} // namespace gfx