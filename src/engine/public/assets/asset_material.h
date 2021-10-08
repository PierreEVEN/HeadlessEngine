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

struct MaterialInfos
{
    // Shader stages
    TAssetPtr<AShader> vertex_stage       = {};
    TAssetPtr<AShader> tessellation_stage = {};
    TAssetPtr<AShader> geometry_stage     = {};
    TAssetPtr<AShader> fragment_stage     = {};

    std::vector<std::string>       renderer_passes = {};
    std::optional<VertexInputInfo> vertex_input    = {};
    PipelineInfos                  pipeline_infos  = {};

    [[nodiscard]] bool                            is_valid() const;
    [[nodiscard]] std::vector<TAssetPtr<AShader>> get_shader_stages() const;
};

class AMaterial : public AssetBase
{
  public:
    AMaterial(const MaterialInfos& in_material_infos);
    virtual ~AMaterial() override = default;

    [[nodiscard]] std::vector<TAssetPtr<AShader>> get_shader_stages() const;
    [[nodiscard]] const std::vector<std::string>& get_used_render_passes() const;
    [[nodiscard]] const MaterialInfos&            get_material_infos() const;
    [[nodiscard]] MaterialPipeline*               get_pipeline(const std::string& render_pass) const;

  private:
    MaterialInfos                                                      material_infos;
    std::unordered_map<std::string, std::unique_ptr<MaterialPipeline>> per_stage_pipeline = {};

    std::vector<VkDescriptorSetLayoutBinding> make_layout_bindings() const;
};