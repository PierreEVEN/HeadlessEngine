#pragma once
#include "asset_base.h"
#include "rendering/shaders/shader_property.h"

class NCamera;
class AMaterial;

struct DescriptorSetsState
{
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
    bool            is_dirty       = true;
};

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
        void mark_descriptor_dirty();
    }

    [[nodiscard]] std::vector<DescriptorSetsState>* get_descriptor_sets(const std::string& render_pass);
    [[nodiscard]] const TAssetPtr<AMaterial>&       get_material_base() const
    {
        return base_material;
    }

    template <typename Property_T = ShaderUserProperty> [[nodiscard]] Property_T* get_property(const std::string& property_name)
    {
        for (const auto& property : shader_properties)
        {
            if (property.base_property.get_property_name() == property_name)
                return property.base_property.get_property_type<Property_T>();
        }

        return nullptr;
    }

  private:
    void mark_descriptor_dirty();

    struct ShaderInstanceProperty
    {
        ShaderUserProperty   base_property;
        VkWriteDescriptorSet write_descriptor_set;
    };

    std::unordered_map<std::string, std::vector<DescriptorSetsState>> descriptor_sets       = {};
    VkDescriptorSetLayout                                             descriptor_set_layout = VK_NULL_HANDLE;

    bool     b_has_vertex_view_buffer           = false;
    bool     b_has_fragment_view_buffer         = false;
    bool     b_has_vertex_transform_buffer      = false;
    bool     b_has_fragment_transform_buffer    = false;
    uint32_t vertex_view_buffer_location        = 0;
    uint32_t fragment_view_buffer_location      = 0;
    uint32_t vertex_transform_buffer_location   = 0;
    uint32_t fragment_transform_buffer_location = 0;

    std::vector<ShaderInstanceProperty> shader_properties;
    TAssetPtr<AMaterial>                base_material;
};