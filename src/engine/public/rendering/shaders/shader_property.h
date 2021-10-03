#pragma once
#include "assets/asset_ptr.h"

#include <any>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

class AShader;

#define SHADER_STATIC_DATA_OBJECT_NAME "SHADER_STATIC_DATA"

class ShaderPropertyTypeBase
{
  public:
    [[nodiscard]] virtual std::string      get_glsl_type_name() const              = 0;
    [[nodiscard]] virtual bool             should_keep_in_buffer_structure() const = 0;
    [[nodiscard]] virtual VkDescriptorType get_descriptor_type() const
    {
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
    [[nodiscard]] virtual VkDescriptorBufferInfo* get_descriptor_buffer_info(const std::any& value, uint32_t image_index) const
    {
        return nullptr;
    }
    [[nodiscard]] virtual VkDescriptorImageInfo* get_descriptor_image_info(const std::any& value, uint32_t image_index) const
    {
        return nullptr;
    }
};

class ShaderUserProperty final
{
  public:
    template <typename Property_T, typename PropertyData_T> [[nodiscard]] static ShaderUserProperty create(const std::string& property_name, const PropertyData_T& default_value)
    {
        ShaderUserProperty property{};
        property.property_type  = std::make_shared<Property_T>();
        property.property_name  = property_name;
        property.property_value = default_value;
        return property;
    }

    template <typename PropertyData_T> PropertyData_T get_value()
    {
        return std::any_cast<PropertyData_T>(property_value);
    }

    template <typename PropertyValue_T> void set_property_value(const PropertyValue_T& value)
    {
        return property_value = value;
    }
    [[nodiscard]] std::string get_property_name() const
    {
        return property_name;
    }
    [[nodiscard]] std::string get_glsl_type_name() const
    {
        return property_type->get_glsl_type_name();
    }
    [[nodiscard]] bool should_keep_in_buffer_structure() const
    {
        return property_type->should_keep_in_buffer_structure();
    }
    [[nodiscard]] VkDescriptorType get_descriptor_type() const
    {
        return property_type->get_descriptor_type();
    }
    [[nodiscard]] VkDescriptorBufferInfo* get_descriptor_buffer_info(uint32_t image_index) const
    {
        return property_type->get_descriptor_buffer_info(property_value, image_index);
    }
    [[nodiscard]] VkDescriptorImageInfo* get_descriptor_image_info(uint32_t image_index) const
    {
        return property_type->get_descriptor_image_info(property_value, image_index);
    }

  protected:
    ShaderUserProperty() = default;

  private:
    std::shared_ptr<ShaderPropertyTypeBase> property_type;
    std::string                             property_name;
    std::any                                property_value;
};

struct ShaderConfiguration
{
    VkShaderStageFlagBits           shader_stage            = VK_SHADER_STAGE_VERTEX_BIT;
    TAssetPtr<AShader>              input_stage             = {};
    bool                            use_view_data_buffer    = false;
    bool                            use_scene_object_buffer = false;
    std::vector<ShaderUserProperty> properties              = {};

    [[nodiscard]] bool        has_buffered_properties() const;
    [[nodiscard]] std::string create_glsl_structure() const;
};

class ShaderPropertyFloat final : public ShaderPropertyTypeBase
{
  public:
    [[nodiscard]] std::string get_glsl_type_name() const override
    {
        return "float";
    }
    [[nodiscard]] bool should_keep_in_buffer_structure() const override
    {
        return true;
    }
};

class ShaderPropertyVec3 final : public ShaderPropertyTypeBase
{
  public:
    [[nodiscard]] std::string get_glsl_type_name() const override
    {
        return "vec3";
    }
    [[nodiscard]] bool should_keep_in_buffer_structure() const override
    {
        return true;
    }
};

class ShaderPropertyVec2 final : public ShaderPropertyTypeBase
{
  public:
    [[nodiscard]] std::string get_glsl_type_name() const override
    {
        return "vec2";
    }
    [[nodiscard]] bool should_keep_in_buffer_structure() const override
    {
        return true;
    }
};