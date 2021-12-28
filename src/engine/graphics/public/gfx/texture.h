#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace gfx
{

enum class EImageFormat
{
    UNDEFINED,
    CHANNEL_R     = 0x00001,
    CHANNEL_G     = 0x00002,
    CHANNEL_B     = 0x00004,
    CHANNEL_A     = 0x00008,
    CHANNEL_RG    = CHANNEL_R | CHANNEL_G,
    CHANNEL_RGB   = CHANNEL_R | CHANNEL_G | CHANNEL_B,
    CHANNEL_RGBA  = CHANNEL_R | CHANNEL_G | CHANNEL_B | CHANNEL_A,
    PRECISION_8   = 0x00010,
    PRECISION_16  = 0x00020,
    PRECISION_32  = 0x00040,
    PRECISION_64  = 0x00080,
    MODE_UNORM    = 0x00100,
    MODE_SRGB     = 0x00200,
    MODE_FLOAT    = 0x00400,
    R_UNORM_8     = MODE_UNORM | PRECISION_8 | CHANNEL_R,
    RG_UNORM_8    = MODE_UNORM | PRECISION_8 | CHANNEL_RG,
    RGB_UNORM_8   = MODE_UNORM | PRECISION_8 | CHANNEL_RGB,
    RGBA_UNORM_8  = MODE_UNORM | PRECISION_8 | CHANNEL_RGBA,
    R_SRGB_8      = MODE_SRGB | PRECISION_8 | CHANNEL_R,
    RG_SRGB_8     = MODE_SRGB | PRECISION_8 | CHANNEL_RG,
    RGB_SRGB_8    = MODE_SRGB | PRECISION_8 | CHANNEL_RGB,
    RGBA_SRGB_8   = MODE_SRGB | PRECISION_8 | CHANNEL_RGBA,
    R_FLOAT_16    = MODE_FLOAT | PRECISION_16 | CHANNEL_R,
    RG_FLOAT_16   = MODE_FLOAT | PRECISION_16 | CHANNEL_RG,
    RGB_FLOAT_16  = MODE_FLOAT | PRECISION_16 | CHANNEL_RGB,
    RGBA_FLOAT_16 = MODE_FLOAT | PRECISION_16 | CHANNEL_RGBA,
    R_FLOAT_32    = MODE_FLOAT | PRECISION_32 | CHANNEL_R,
    RG_FLOAT_32   = MODE_FLOAT | PRECISION_32 | CHANNEL_RG,
    RGB_FLOAT_32  = MODE_FLOAT | PRECISION_32 | CHANNEL_RGB,
    RGBA_FLOAT_32 = MODE_FLOAT | PRECISION_32 | CHANNEL_RGBA,
};

enum class EImageType
{
    Texture_1D,
    Texture_1D_Array,
    Texture_2D,
    Texture_2D_Array,
    Texture_3D,
    Cubemap,
};

enum class ETextureTransferCapabilities
{
    CopySource      = 0x00001,
    CopyDestination = 0x00002,
};

enum class ETextureGPUWriteCapabilities
{
    None,
    ColorWrite,
    DepthStencilWrite
};

enum class ETextureGPUReadCapabilities
{
    None,
    Sampling,
};
struct TextureParameter
{
    EImageFormat                 format                 = EImageFormat::RGBA_UNORM_8;
    EImageType                   image_type             = EImageType::Texture_2D;
    ETextureTransferCapabilities transfer_capabilities  = ETextureTransferCapabilities::CopyDestination;
    ETextureGPUWriteCapabilities gpu_write_capabilities = ETextureGPUWriteCapabilities::None;
    ETextureGPUReadCapabilities  gpu_read_capabilities  = ETextureGPUReadCapabilities::Sampling;
    std::optional<uint32_t>      mip_level;
};

class Texture
{
  public:
    [[nodiscard]] static std::shared_ptr<Texture> create(uint32_t width, uint32_t height, uint32_t depth, const TextureParameter& parameters = {});
    [[nodiscard]] static std::shared_ptr<Texture> create(uint32_t width, uint32_t height, const TextureParameter& parameters = {})
    {
        return create(width, height, 1, parameters);
    }
    virtual ~Texture() = default;

    virtual void set_pixels(const std::vector<uint8_t>& data) = 0;

    [[nodiscard]] uint8_t  get_channel_count();
    [[nodiscard]] uint8_t  get_byte_per_pixel();
    [[nodiscard]] uint32_t get_data_size();

  protected:
    Texture(uint32_t pixel_width, uint32_t pixel_height, uint32_t pixel_depth, const TextureParameter& parameters);

  protected:
    TextureParameter image_parameters;
    uint32_t         width;
    uint32_t         height;
    uint32_t         depth;
};
} // namespace gfx