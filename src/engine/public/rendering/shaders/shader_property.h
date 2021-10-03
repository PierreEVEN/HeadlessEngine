#pragma once
#include "assets/asset_ptr.h"

#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

class AShader;

#define SHADER_STATIC_DATA_OBJECT_NAME "SHADER_STATIC_DATA"

class ShaderUserProperty
{
  public:
    template <typename Property_T, typename PropertyData_T> [[nodiscard]] static std::shared_ptr<ShaderUserProperty> create(const std::string& property_name, const PropertyData_T& data)
    {
        Property_T* property_memory    = new Property_T(data);
        property_memory->property_name = property_name;
        return std::shared_ptr<Property_T>(property_memory);
    }

    [[nodiscard]] virtual std::string      get_property_name() const;
    [[nodiscard]] virtual std::string      get_glsl_type_name() const              = 0;
    [[nodiscard]] virtual bool             should_keep_in_buffer_structure() const = 0;
    [[nodiscard]] virtual VkDescriptorType get_descriptor_type() const
    {
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

  private:
    std::string property_name;
};

struct ShaderConfiguration
{
    VkShaderStageFlagBits                            shader_stage            = VK_SHADER_STAGE_VERTEX_BIT;
    TAssetPtr<AShader>                               input_stage             = {};
    bool                                             use_view_data_buffer    = false;
    bool                                             use_scene_object_buffer = false;
    std::vector<std::shared_ptr<ShaderUserProperty>> properties              = {};

    [[nodiscard]] bool        has_buffered_properties() const;
    [[nodiscard]] std::string create_glsl_structure() const;
};

class ShaderPropertyFloat final : public ShaderUserProperty
{
  public:
    ShaderPropertyFloat(float in_value) : value(in_value)
    {
    }

    [[nodiscard]] std::string get_glsl_type_name() const override
    {
        return "float";
    }
    [[nodiscard]] bool should_keep_in_buffer_structure() const override
    {
        return true;
    }

  private:
    float value;
};

class ShaderPropertyVec3 final : public ShaderUserProperty
{
  public:
    ShaderPropertyVec3(const glm::vec3& in_value) : value(in_value)
    {
    }

    [[nodiscard]] std::string get_glsl_type_name() const override
    {
        return "vec3";
    }
    [[nodiscard]] bool should_keep_in_buffer_structure() const override
    {
        return true;
    }

  private:
    glm::vec3 value;
};

class ShaderPropertyVec2 final : public ShaderUserProperty
{
  public:
    ShaderPropertyVec2(const glm::vec2& in_value) : value(in_value)
    {
    }

    [[nodiscard]] std::string get_glsl_type_name() const override
    {
        return "vec2";
    }
    [[nodiscard]] bool should_keep_in_buffer_structure() const override
    {
        return true;
    }

  private:
    glm::vec2 value;
};