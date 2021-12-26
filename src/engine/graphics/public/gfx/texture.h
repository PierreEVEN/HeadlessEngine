#pragma once

#include <cstdint>

namespace gfx
{

enum class ImageFormat
{
    UNDEFINED,
    R_UNORM_8,
    RG_UNORM_8,
    RGB_UNORM_8,
    RGBA_UNORM_8,
    R_SRGB_8,
    RG_SRGB_8,
    RGB_SRGB_8,
    RGBA_SRGB_8,
    R_FLOAT_16,
    RG_FLOAT_16,
    RGB_FLOAT_16,
    RGBA_FLOAT_16,
    R_FLOAT_32,
    RG_FLOAT_32,
    RGB_FLOAT_32,
    RGBA_FLOAT_32,
};

class Texture
{
public:
    Texture(uint32_t width, uint32_t height, ImageFormat format)
    {
        
    }

    Texture(uint32_t width, uint32_t height, uint32_t depth, ImageFormat format)
    {
    }


};
} // namespace gfx