#pragma once

#include "gfx/types.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace gfx
{
struct TextureParameter
{
    ETypeFormat                  format                 = ETypeFormat::R8G8B8A8_UNORM;
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

    [[nodiscard]] uint32_t get_data_size();

    static bool is_depth_format(ETypeFormat format)
    {
        return format == ETypeFormat::D24_UNORM_S8_UINT || format == ETypeFormat::D32_SFLOAT || format == ETypeFormat::D32_SFLOAT_S8_UINT;
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