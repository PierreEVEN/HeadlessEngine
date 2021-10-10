#pragma once
#include "assets/asset_ptr.h"
#include "rendering/mesh/vertex.h"

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <boost/pfr.hpp>

class AShaderBuffer;
class ATexture;
class AShader;

#define SHADER_STATIC_DATA_OBJECT_NAME "SHADER_STATIC_DATA"

struct TextureProperty
{
    std::string         binding_name = "";
    TAssetPtr<ATexture> texture = {};
};

class BufferProperty final
{
  public:
    [[nodiscard]] static BufferProperty create(const std::string& property_name, const TAssetPtr<AShaderBuffer>& default_value)
    {
        BufferProperty property{};
        property.property_data = default_value;
        property.property_name = property_name;
        return property;
    }

    [[nodiscard]] std::string get_property_name() const
    {
        return property_name;
    }

  protected:
    BufferProperty() = default;

  private:
    std::string              property_name = "";
    TAssetPtr<AShaderBuffer> property_data = {};
};

struct ShaderInfos
{
    VkShaderStageFlagBits          shader_stage            = VK_SHADER_STAGE_VERTEX_BIT;
    std::optional<VertexInputInfo> vertex_inputs           = {};
    bool                           use_view_data_buffer    = false;
    bool                           use_scene_object_buffer = false;
    std::vector<TextureProperty>   textures                = {};
    std::vector<BufferProperty>    buffers                 = {};
};