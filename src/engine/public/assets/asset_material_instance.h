#pragma once
#include "asset_base.h"
#include "rendering/shaders/shader_property.h"

class AMaterial;

class AMaterialInstance : public AssetBase
{
  public:
    AMaterialInstance(const TAssetPtr<AMaterial>& in_base_material);

    //@TODO
    void update_descriptor_sets()
    {
    }

    template<typename PropertyValue_T>
    void set_property_value(std::string property_name, const PropertyValue_T& value_type)
    {
        for (const auto& property : shader_properties)
        {
            if (property_name == property.get_property_name())
            {
                property.set_property_value(value_type);
                return;
            }
        }
        LOG_WARNING("failed to find property %s on material instance %s", property_name.c_str(), to_string().c_str());
    }

  private:
    std::vector<ShaderUserProperty> shader_properties;
    TAssetPtr<AMaterial>            base_material;
};