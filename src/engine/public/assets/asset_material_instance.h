#pragma once
#include "asset_base.h"
#include "rendering/shaders/shader_property.h"

class NCamera;
class AMaterial;

class AMaterialInstance : public AssetBase
{
  public:
    AMaterialInstance(const TAssetPtr<AMaterial>& in_base_material);

    void update_descriptor_sets(const std::string& render_pass, NCamera* in_camera, uint32_t imageIndex);

    template <typename PropertyValue_T> void set_property_value(std::string property_name, const PropertyValue_T& value_type)
    {
        for (const auto& property : shader_properties)
        {
            if (property_name == property.base_property.get_property_name())
            {
                property.base_property.set_property_value(value_type);
                return;
            }
        }
        LOG_WARNING("failed to find property %s on material instance %s", property_name.c_str(), to_string().c_str());
    }

    [[nodiscard]] VkPipelineLayout                    get_pipeline_layout(const std::string& render_pass) const;
    [[nodiscard]] VkPipeline                          get_pipeline(const std::string& render_pass) const;
    [[nodiscard]] const std::vector<VkDescriptorSet>& get_descriptor_sets(const std::string& render_pass) const;

  private:
    struct ShaderInstanceProperty
    {
        ShaderUserProperty   base_property;
        VkWriteDescriptorSet write_descriptor_set;
    };

    std::vector<ShaderInstanceProperty> shader_properties;
    TAssetPtr<AMaterial>                base_material;
};