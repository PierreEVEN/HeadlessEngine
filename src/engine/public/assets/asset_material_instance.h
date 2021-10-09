#pragma once
#include "asset_base.h"
#include "rendering/shaders/shader_property.h"

class NCamera;
class AMaterial;

struct DescriptorSetsState
{
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};

class AMaterialInstance : public AssetBase
{
  public:
    AMaterialInstance(const TAssetPtr<AMaterial>& in_base_material);

    void update_descriptor_sets(const std::string& render_pass, NCamera* in_camera, uint32_t imageIndex);

    [[nodiscard]] std::vector<DescriptorSetsState>* get_descriptor_sets(const std::string& render_pass);
    [[nodiscard]] const TAssetPtr<AMaterial>&       get_material_base() const
    {
        return base_material;
    }

    void set_texture(const std::string& property_name, const TAssetPtr<ATexture>& in_texture)
    {
        for (auto& property : textures)
            if (property.base_property.binding_name == property_name)
                property.base_property.texture = in_texture;
    }

  private:
    struct TextureRuntimeProperty
    {
        TextureProperty      base_property;
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

    std::vector<TextureRuntimeProperty> textures;
    TAssetPtr<AMaterial>                base_material;
};