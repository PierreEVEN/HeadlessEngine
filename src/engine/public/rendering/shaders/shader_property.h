#pragma once
#include "assets/asset_ptr.h"
#include "rendering/mesh/vertex.h"

#include <cpputils/logger.hpp>
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
    TAssetPtr<ATexture> texture      = TAssetPtr<ATexture>("default_texture");
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
    template <typename Type_T> [[nodiscard]] static PushConstant create()
    {
        Type_T type_inst;

        PushConstant pc{};
        pc.members   = type_inst.get_members();
        pc.data_size = 0;
        for (const auto& member : pc.members)
            pc.data_size += member.property_size;
        pc.data      = malloc(pc.data_size);
        return pc;
    }

    PushConstant() = default;

    PushConstant(PushConstant&& other)
    {
        copy_from(other);
    }

    PushConstant(const PushConstant& other)
    {
        copy_from(other);
    }

    ~PushConstant()
    {
        if (data)
            free(data);
    }

    void operator=(const PushConstant& other)
    {
        copy_from(other);
    }

    template <typename Structure_T> [[nodiscard]] Structure_T& get()
    {
        if (sizeof(Structure_T) != data_size)
            LOG_FATAL("push_constant size doesn't match expected size (expected : %d, value = %d)", data_size, sizeof(Structure_T));

        return *static_cast<Structure_T*>(data);
    }

    [[nodiscard]] size_t get_range() const
    {
        return data_size;
    }

    struct Property final
    {
        template <typename Type_T> [[nodiscard]] static Property create(const std::string& glsl_type, const std::string& name)
        {
            Property pcp{};
            pcp.property_size = sizeof(Type_T);
            pcp.type_name     = glsl_type;
            pcp.property_name = name;
            return pcp;
        }

        size_t      property_size;
        std::string type_name;
        std::string property_name;
    };

    [[nodiscard]] const std::vector<Property>& get_members() const
    {
        return members;
    }

    class Type
    {
      public:
        [[nodiscard]] virtual std::vector<Property> get_members() = 0;
    };

  private:
    void copy_from(const PushConstant& other)
    {
        members = other.members;
        data_size = other.data_size;
        data = malloc(other.data_size);
        memcpy(data, other.data, other.data_size);
    }

    std::vector<Property> members   = {};
    size_t                data_size = 0;
    void*                 data      = nullptr;
};

struct ShaderInfos
{
    VkShaderStageFlagBits          shader_stage            = VK_SHADER_STAGE_VERTEX_BIT;
    std::optional<VertexInputInfo> vertex_inputs_override           = {};
    bool                           use_view_data_buffer    = false;
    bool                           use_scene_object_buffer = false;
    std::vector<TextureProperty>   textures                = {};
    std::vector<BufferProperty>    buffers                 = {};
    std::optional<PushConstant>    push_constants          = {};
};