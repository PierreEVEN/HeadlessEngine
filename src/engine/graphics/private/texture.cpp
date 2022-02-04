
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
    if (parameters.format == ETypeFormat::UNDEFINED)
        LOG_FATAL("buffer format is undefined");

    switch (image_parameters.image_type)
    {
    case EImageType::Texture_1D:
        width  = pixel_width;
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
    return std::make_shared<vulkan::Texture_VK>(width, height, depth, parameters);
#else
    static_assert(false, "backend not supported");
#endif
}


uint32_t Texture::get_data_size()
{
    return width * height * depth * get_format_channel_count(image_parameters.format) * get_format_bytes_per_pixel(image_parameters.format);
}
} // namespace gfx