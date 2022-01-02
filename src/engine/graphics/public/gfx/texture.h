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
    CHANNEL_R                = 0x00000001,
    CHANNEL_G                = 0x0000002,
    CHANNEL_B                = 0x0000004,
    CHANNEL_A                = 0x0000008,
    PRECISION_8              = 0x0000010,
    PRECISION_16             = 0x0000020,
    PRECISION_32             = 0x0000040,
    PRECISION_64             = 0x0000080,
    MODE_UNORM               = 0x0000100,
    MODE_SRGB                = 0x0000200,
    MODE_FLOAT               = 0x0000400,
    BGRA_UNORM_8             = 0x0001000,
    DEPTH_32_FLOAT           = 0x0002000,
    DEPTH_32_STENCIL_8_FLOAT = 0x0004000,
    DEPTH_24_STENCIL_8_UNORM = 0x0008000,
    CHANNEL_RG               = CHANNEL_R | CHANNEL_G,
    CHANNEL_RGB              = CHANNEL_R | CHANNEL_G | CHANNEL_B,
    CHANNEL_RGBA             = CHANNEL_R | CHANNEL_G | CHANNEL_B | CHANNEL_A,
    R_UNORM_8                = MODE_UNORM | PRECISION_8 | CHANNEL_R,
    RG_UNORM_8               = MODE_UNORM | PRECISION_8 | CHANNEL_RG,
    RGB_UNORM_8              = MODE_UNORM | PRECISION_8 | CHANNEL_RGB,
    RGBA_UNORM_8             = MODE_UNORM | PRECISION_8 | CHANNEL_RGBA,
    R_SRGB_8                 = MODE_SRGB | PRECISION_8 | CHANNEL_R,
    RG_SRGB_8                = MODE_SRGB | PRECISION_8 | CHANNEL_RG,
    RGB_SRGB_8               = MODE_SRGB | PRECISION_8 | CHANNEL_RGB,
    RGBA_SRGB_8              = MODE_SRGB | PRECISION_8 | CHANNEL_RGBA,
    R_FLOAT_16               = MODE_FLOAT | PRECISION_16 | CHANNEL_R,
    RG_FLOAT_16              = MODE_FLOAT | PRECISION_16 | CHANNEL_RG,
    RGB_FLOAT_16             = MODE_FLOAT | PRECISION_16 | CHANNEL_RGB,
    RGBA_FLOAT_16            = MODE_FLOAT | PRECISION_16 | CHANNEL_RGBA,
    R_FLOAT_32               = MODE_FLOAT | PRECISION_32 | CHANNEL_R,
    RG_FLOAT_32              = MODE_FLOAT | PRECISION_32 | CHANNEL_RG,
    RGB_FLOAT_32             = MODE_FLOAT | PRECISION_32 | CHANNEL_RGB,
    RGBA_FLOAT_32            = MODE_FLOAT | PRECISION_32 | CHANNEL_RGBA,
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
    None            = 0x00000,
    CopySource      = 0x00001,
    CopyDestination = 0x00002,
};

enum class ETextureGPUWriteCapabilities
{
    None,
    Enabled,
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
    bool                         read_only = true;
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

    static bool is_depth_format(EImageFormat format)
    {
        return format == EImageFormat::DEPTH_24_STENCIL_8_UNORM || format == EImageFormat::DEPTH_32_FLOAT || format == EImageFormat::DEPTH_32_STENCIL_8_FLOAT;
    }
    
  protected:
    Texture(uint32_t pixel_width, uint32_t pixel_height, uint32_t pixel_depth, const TextureParameter& parameters);


  protected:
    TextureParameter image_parameters;
    uint32_t         width;
    uint32_t         height;
    uint32_t         depth;
};
} // namespace gfx