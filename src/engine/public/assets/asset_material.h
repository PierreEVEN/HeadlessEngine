#pragma once
#include "asset_base.h"
#include "asset_mesh_data.h"
#include "asset_shader.h"
#include "rendering/vulkan/descriptor_pool.h"
#include "rendering/vulkan/material_pipeline.h"

class AShaderBuffer;
class ATexture2D;
class AShader;
class NCamera;

class AMaterial : public AssetBase
{
  public:
    AMaterial(const TAssetPtr<AShader>& final_shader_stage, const std::vector<std::string>& use_with_render_passes, MaterialPipelineConfiguration pipeline_configuration = {},
              std::optional<VertexInputConfig> vertex_input_override = {});
    virtual ~AMaterial() override = default;

    [[nodiscard]] VkPipelineLayout                    get_pipeline_layout(const std::string& render_pass) const;
    [[nodiscard]] VkPipeline                          get_pipeline(const std::string& render_pass) const;
    [[nodiscard]] const std::vector<VkDescriptorSet>& get_descriptor_sets(const std::string& render_pass) const;
    [[nodiscard]] std::vector<TAssetPtr<AShader>>     get_shader_stages() const;

    void update_descriptor_sets(const std::string& render_pass, NCamera* in_scene, uint32_t imageIndex);

    void set_property_value();

  private:
    TAssetPtr<AShader>                                final_stage        = {};
    std::unordered_map<std::string, MaterialPipeline> per_stage_pipeline = {};

    [[nodiscard]] const MaterialPipeline& get_pipeline_class(const std::string& render_pass) const;
    MaterialPipelineBindings              make_layout_bindings();

    void add_layout_binding(std::vector<VkDescriptorSetLayoutBinding>& bindings, const ShaderReflectProperty* property, VkShaderStageFlags shader_stage, VkDescriptorType descriptor_type);
};