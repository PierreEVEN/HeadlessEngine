
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

uint8_t Texture::get_format_channel_count(ETypeFormat format)
{
    switch (format)
    {
    case ETypeFormat::UNDEFINED:
        return 0;
    case ETypeFormat::R8_UNORM:
        return 1;
    case ETypeFormat::R8_SNORM:
        return 1;
    case ETypeFormat::R8_USCALED:
        return 1;
    case ETypeFormat::R8_SSCALED:
        return 1;
    case ETypeFormat::R8_UINT:
        return 1;
    case ETypeFormat::R8_SINT:
        return 1;
    case ETypeFormat::R8_SRGB:
        return 1;
    case ETypeFormat::R8G8_UNORM:
        return 2;
    case ETypeFormat::R8G8_SNORM:
        return 2;
    case ETypeFormat::R8G8_USCALED:
        return 2;
    case ETypeFormat::R8G8_SSCALED:
        return 2;
    case ETypeFormat::R8G8_UINT:
        return 2;
    case ETypeFormat::R8G8_SINT:
        return 2;
    case ETypeFormat::R8G8_SRGB:
        return 2;
    case ETypeFormat::R8G8B8_UNORM:
        return 3;
    case ETypeFormat::R8G8B8_SNORM:
        return 3;
    case ETypeFormat::R8G8B8_USCALED:
        return 3;
    case ETypeFormat::R8G8B8_SSCALED:
        return 3;
    case ETypeFormat::R8G8B8_UINT:
        return 3;
    case ETypeFormat::R8G8B8_SINT:
        return 3;
    case ETypeFormat::R8G8B8_SRGB:
        return 3;
    case ETypeFormat::B8G8R8_UNORM:
        return 3;
    case ETypeFormat::B8G8R8_SNORM:
        return 3;
    case ETypeFormat::B8G8R8_USCALED:
        return 3;
    case ETypeFormat::B8G8R8_SSCALED:
        return 3;
    case ETypeFormat::B8G8R8_UINT:
        return 3;
    case ETypeFormat::B8G8R8_SINT:
        return 3;
    case ETypeFormat::B8G8R8_SRGB:
        return 3;
    case ETypeFormat::R8G8B8A8_UNORM:
        return 4;
    case ETypeFormat::R8G8B8A8_SNORM:
        return 4;
    case ETypeFormat::R8G8B8A8_USCALED:
        return 4;
    case ETypeFormat::R8G8B8A8_SSCALED:
        return 4;
    case ETypeFormat::R8G8B8A8_UINT:
        return 4;
    case ETypeFormat::R8G8B8A8_SINT:
        return 4;
    case ETypeFormat::R8G8B8A8_SRGB:
        return 4;
    case ETypeFormat::B8G8R8A8_UNORM:
        return 4;
    case ETypeFormat::B8G8R8A8_SNORM:
        return 4;
    case ETypeFormat::B8G8R8A8_USCALED:
        return 4;
    case ETypeFormat::B8G8R8A8_SSCALED:
        return 4;
    case ETypeFormat::B8G8R8A8_UINT:
        return 4;
    case ETypeFormat::B8G8R8A8_SINT:
        return 4;
    case ETypeFormat::B8G8R8A8_SRGB:
        return 4;
    case ETypeFormat::A8B8G8R8_UNORM_PACK32:
        return 4;
    case ETypeFormat::A8B8G8R8_SNORM_PACK32:
        return 4;
    case ETypeFormat::A8B8G8R8_USCALED_PACK32:
        return 4;
    case ETypeFormat::A8B8G8R8_SSCALED_PACK32:
        return 4;
    case ETypeFormat::A8B8G8R8_UINT_PACK32:
        return 4;
    case ETypeFormat::A8B8G8R8_SINT_PACK32:
        return 4;
    case ETypeFormat::A8B8G8R8_SRGB_PACK32:
        return 4;
    case ETypeFormat::R16_UNORM:
        return 1;
    case ETypeFormat::R16_SNORM:
        return 1;
    case ETypeFormat::R16_USCALED:
        return 1;
    case ETypeFormat::R16_SSCALED:
        return 1;
    case ETypeFormat::R16_UINT:
        return 1;
    case ETypeFormat::R16_SINT:
        return 1;
    case ETypeFormat::R16_SFLOAT:
        return 1;
    case ETypeFormat::R16G16_UNORM:
        return 2;
    case ETypeFormat::R16G16_SNORM:
        return 2;
    case ETypeFormat::R16G16_USCALED:
        return 2;
    case ETypeFormat::R16G16_SSCALED:
        return 2;
    case ETypeFormat::R16G16_UINT:
        return 2;
    case ETypeFormat::R16G16_SINT:
        return 2;
    case ETypeFormat::R16G16_SFLOAT:
        return 2;
    case ETypeFormat::R16G16B16_UNORM:
        return 3;
    case ETypeFormat::R16G16B16_SNORM:
        return 3;
    case ETypeFormat::R16G16B16_USCALED:
        return 3;
    case ETypeFormat::R16G16B16_SSCALED:
        return 3;
    case ETypeFormat::R16G16B16_UINT:
        return 3;
    case ETypeFormat::R16G16B16_SINT:
        return 3;
    case ETypeFormat::R16G16B16_SFLOAT:
        return 3;
    case ETypeFormat::R16G16B16A16_UNORM:
        return 4;
    case ETypeFormat::R16G16B16A16_SNORM:
        return 4;
    case ETypeFormat::R16G16B16A16_USCALED:
        return 4;
    case ETypeFormat::R16G16B16A16_SSCALED:
        return 4;
    case ETypeFormat::R16G16B16A16_UINT:
        return 4;
    case ETypeFormat::R16G16B16A16_SINT:
        return 4;
    case ETypeFormat::R16G16B16A16_SFLOAT:
        return 4;
    case ETypeFormat::R32_UINT:
        return 1;
    case ETypeFormat::R32_SINT:
        return 1;
    case ETypeFormat::R32_SFLOAT:
        return 1;
    case ETypeFormat::R32G32_UINT:
        return 2;
    case ETypeFormat::R32G32_SINT:
        return 2;
    case ETypeFormat::R32G32_SFLOAT:
        return 2;
    case ETypeFormat::R32G32B32_UINT:
        return 3;
    case ETypeFormat::R32G32B32_SINT:
        return 3;
    case ETypeFormat::R32G32B32_SFLOAT:
        return 3;
    case ETypeFormat::R32G32B32A32_UINT:
        return 4;
    case ETypeFormat::R32G32B32A32_SINT:
        return 4;
    case ETypeFormat::R32G32B32A32_SFLOAT:
        return 4;
    case ETypeFormat::R64_UINT:
        return 1;
    case ETypeFormat::R64_SINT:
        return 1;
    case ETypeFormat::R64_SFLOAT:
        return 1;
    case ETypeFormat::R64G64_UINT:
        return 2;
    case ETypeFormat::R64G64_SINT:
        return 2;
    case ETypeFormat::R64G64_SFLOAT:
        return 2;
    case ETypeFormat::R64G64B64_UINT:
        return 3;
    case ETypeFormat::R64G64B64_SINT:
        return 3;
    case ETypeFormat::R64G64B64_SFLOAT:
        return 3;
    case ETypeFormat::R64G64B64A64_UINT:
        return 4;
    case ETypeFormat::R64G64B64A64_SINT:
        return 4;
    case ETypeFormat::R64G64B64A64_SFLOAT:
        return 4;
    case ETypeFormat::D16_UNORM:
        return 0;
    case ETypeFormat::D32_SFLOAT:
        return 0;
    case ETypeFormat::D16_UNORM_S8_UINT:
        return 0;
    case ETypeFormat::D24_UNORM_S8_UINT:
        return 0;
    case ETypeFormat::D32_SFLOAT_S8_UINT:
        return 0;
    default:
        return 0;
    }
}

uint8_t Texture::get_format_bytes_per_pixel(ETypeFormat format)
{
    switch (format)
    {
    case ETypeFormat::UNDEFINED:
        return 0;
    case ETypeFormat::R8_UNORM:
        return 1;
    case ETypeFormat::R8_SNORM:
        return 1;
    case ETypeFormat::R8_USCALED:
        return 1;
    case ETypeFormat::R8_SSCALED:
        return 1;
    case ETypeFormat::R8_UINT:
        return 1;
    case ETypeFormat::R8_SINT:
        return 1;
    case ETypeFormat::R8_SRGB:
        return 1;
    case ETypeFormat::R8G8_UNORM:
        return 1;
    case ETypeFormat::R8G8_SNORM:
        return 1;
    case ETypeFormat::R8G8_USCALED:
        return 1;
    case ETypeFormat::R8G8_SSCALED:
        return 1;
    case ETypeFormat::R8G8_UINT:
        return 1;
    case ETypeFormat::R8G8_SINT:
        return 1;
    case ETypeFormat::R8G8_SRGB:
        return 1;
    case ETypeFormat::R8G8B8_UNORM:
        return 1;
    case ETypeFormat::R8G8B8_SNORM:
        return 1;
    case ETypeFormat::R8G8B8_USCALED:
        return 1;
    case ETypeFormat::R8G8B8_SSCALED:
        return 1;
    case ETypeFormat::R8G8B8_UINT:
        return 1;
    case ETypeFormat::R8G8B8_SINT:
        return 1;
    case ETypeFormat::R8G8B8_SRGB:
        return 1;
    case ETypeFormat::B8G8R8_UNORM:
        return 1;
    case ETypeFormat::B8G8R8_SNORM:
        return 1;
    case ETypeFormat::B8G8R8_USCALED:
        return 1;
    case ETypeFormat::B8G8R8_SSCALED:
        return 1;
    case ETypeFormat::B8G8R8_UINT:
        return 1;
    case ETypeFormat::B8G8R8_SINT:
        return 1;
    case ETypeFormat::B8G8R8_SRGB:
        return 1;
    case ETypeFormat::R8G8B8A8_UNORM:
        return 1;
    case ETypeFormat::R8G8B8A8_SNORM:
        return 1;
    case ETypeFormat::R8G8B8A8_USCALED:
        return 1;
    case ETypeFormat::R8G8B8A8_SSCALED:
        return 1;
    case ETypeFormat::R8G8B8A8_UINT:
        return 1;
    case ETypeFormat::R8G8B8A8_SINT:
        return 1;
    case ETypeFormat::R8G8B8A8_SRGB:
        return 1;
    case ETypeFormat::B8G8R8A8_UNORM:
        return 1;
    case ETypeFormat::B8G8R8A8_SNORM:
        return 1;
    case ETypeFormat::B8G8R8A8_USCALED:
        return 1;
    case ETypeFormat::B8G8R8A8_SSCALED:
        return 1;
    case ETypeFormat::B8G8R8A8_UINT:
        return 1;
    case ETypeFormat::B8G8R8A8_SINT:
        return 1;
    case ETypeFormat::B8G8R8A8_SRGB:
        return 1;
    case ETypeFormat::A8B8G8R8_UNORM_PACK32:
        return 1;
    case ETypeFormat::A8B8G8R8_SNORM_PACK32:
        return 1;
    case ETypeFormat::A8B8G8R8_USCALED_PACK32:
        return 1;
    case ETypeFormat::A8B8G8R8_SSCALED_PACK32:
        return 1;
    case ETypeFormat::A8B8G8R8_UINT_PACK32:
        return 1;
    case ETypeFormat::A8B8G8R8_SINT_PACK32:
        return 1;
    case ETypeFormat::A8B8G8R8_SRGB_PACK32:
        return 1;
    case ETypeFormat::R16_UNORM:
        return 2;
    case ETypeFormat::R16_SNORM:
        return 2;
    case ETypeFormat::R16_USCALED:
        return 2;
    case ETypeFormat::R16_SSCALED:
        return 2;
    case ETypeFormat::R16_UINT:
        return 2;
    case ETypeFormat::R16_SINT:
        return 2;
    case ETypeFormat::R16_SFLOAT:
        return 2;
    case ETypeFormat::R16G16_UNORM:
        return 2;
    case ETypeFormat::R16G16_SNORM:
        return 2;
    case ETypeFormat::R16G16_USCALED:
        return 2;
    case ETypeFormat::R16G16_SSCALED:
        return 2;
    case ETypeFormat::R16G16_UINT:
        return 2;
    case ETypeFormat::R16G16_SINT:
        return 2;
    case ETypeFormat::R16G16_SFLOAT:
        return 2;
    case ETypeFormat::R16G16B16_UNORM:
        return 2;
    case ETypeFormat::R16G16B16_SNORM:
        return 2;
    case ETypeFormat::R16G16B16_USCALED:
        return 2;
    case ETypeFormat::R16G16B16_SSCALED:
        return 2;
    case ETypeFormat::R16G16B16_UINT:
        return 2;
    case ETypeFormat::R16G16B16_SINT:
        return 2;
    case ETypeFormat::R16G16B16_SFLOAT:
        return 2;
    case ETypeFormat::R16G16B16A16_UNORM:
        return 2;
    case ETypeFormat::R16G16B16A16_SNORM:
        return 2;
    case ETypeFormat::R16G16B16A16_USCALED:
        return 2;
    case ETypeFormat::R16G16B16A16_SSCALED:
        return 2;
    case ETypeFormat::R16G16B16A16_UINT:
        return 2;
    case ETypeFormat::R16G16B16A16_SINT:
        return 2;
    case ETypeFormat::R16G16B16A16_SFLOAT:
        return 2;
    case ETypeFormat::R32_UINT:
        return 4;
    case ETypeFormat::R32_SINT:
        return 4;
    case ETypeFormat::R32_SFLOAT:
        return 4;
    case ETypeFormat::R32G32_UINT:
        return 4;
    case ETypeFormat::R32G32_SINT:
        return 4;
    case ETypeFormat::R32G32_SFLOAT:
        return 4;
    case ETypeFormat::R32G32B32_UINT:
        return 4;
    case ETypeFormat::R32G32B32_SINT:
        return 4;
    case ETypeFormat::R32G32B32_SFLOAT:
        return 4;
    case ETypeFormat::R32G32B32A32_UINT:
        return 4;
    case ETypeFormat::R32G32B32A32_SINT:
        return 4;
    case ETypeFormat::R32G32B32A32_SFLOAT:
        return 4;
    case ETypeFormat::R64_UINT:
        return 8;
    case ETypeFormat::R64_SINT:
        return 8;
    case ETypeFormat::R64_SFLOAT:
        return 8;
    case ETypeFormat::R64G64_UINT:
        return 8;
    case ETypeFormat::R64G64_SINT:
        return 8;
    case ETypeFormat::R64G64_SFLOAT:
        return 8;
    case ETypeFormat::R64G64B64_UINT:
        return 8;
    case ETypeFormat::R64G64B64_SINT:
        return 8;
    case ETypeFormat::R64G64B64_SFLOAT:
        return 8;
    case ETypeFormat::R64G64B64A64_UINT:
        return 8;
    case ETypeFormat::R64G64B64A64_SINT:
        return 8;
    case ETypeFormat::R64G64B64A64_SFLOAT:
        return 8;
    default:
        return 0;
    }
}

uint32_t Texture::get_data_size()
{
    return width * height * depth * get_format_channel_count(image_parameters.format) * get_format_bytes_per_pixel(image_parameters.format);
}
} // namespace gfx