#pragma once
#include "assets/asset_ptr.h"
#include "rendering/mesh/vertex.h"

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

class AShaderBuffer;
class ATexture;
class AShader;

#define SHADER_STATIC_DATA_OBJECT_NAME "SHADER_STATIC_DATA"

struct TextureProperty
{
    std::string         binding_name = "";
    TAssetPtr<ATexture> texture      = {};
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

class PushConstant final
{
  public:
    template <typename Structure_T> PushConstant()
    {
        data_size = sizeof(Structure_T);
        data      = malloc(data_size);
    }

    template <typename Structure_T>
    [[nodiscard]] Structure_T& get()
    {
        return *dynamic_cast<Structure_T*>(data);
    }

    [[nodiscard]] size_t get_range() const
    {
        return data_size;
    }

  private:
    size_t data_size = 0;
    void*  data      = nullptr;
};

struct ShaderInfos
{
    VkShaderStageFlagBits          shader_stage            = VK_SHADER_STAGE_VERTEX_BIT;
    std::optional<VertexInputInfo> vertex_inputs           = {};
    bool                           use_view_data_buffer    = false;
    bool                           use_scene_object_buffer = false;
    std::vector<TextureProperty>   textures                = {};
    std::vector<BufferProperty>    buffers                 = {};
    std::optional<PushConstant>    push_constants          = {};
};