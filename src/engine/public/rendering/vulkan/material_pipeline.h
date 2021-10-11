#pragma once

#include "assets/asset_ptr.h"
#include "rendering/mesh/vertex.h"
#include "rendering/shaders/shader_property.h"

#include <optional>
#include <vulkan/vulkan.h>

class AShader;

struct PipelineInfos
{
    // Pipeline config
    VkBool32             depth_test            = VK_TRUE;
    VkBool32             wireframe             = VK_FALSE;
    std::optional<float> wireframe_lines_width = {};
    VkPrimitiveTopology  topology              = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode        polygon_mode          = VK_POLYGON_MODE_FILL;
    bool                 is_translucent        = false;
    bool                 backface_culling      = true;
};

struct MaterialInfos
{
    // Shader stages
    TAssetPtr<AShader> vertex_stage       = {};
    TAssetPtr<AShader> tessellation_stage = {};
    TAssetPtr<AShader> geometry_stage     = {};
    TAssetPtr<AShader> fragment_stage     = {};

    std::vector<std::string> renderer_passes = {};
    PipelineInfos            pipeline_infos  = {};

    [[nodiscard]] bool                            is_valid() const;
    [[nodiscard]] std::vector<TAssetPtr<AShader>> get_shader_stages() const;
};

class MaterialPipeline final
{
  public:
    MaterialPipeline(const MaterialInfos& material_infos, const std::string& render_pass, const std::vector<VkDescriptorSetLayoutBinding>& layout_bindings);
    ~MaterialPipeline();

    [[nodiscard]] VkPipelineLayout*      get_pipeline_layout();
    [[nodiscard]] VkPipeline             get_pipeline() const;
    [[nodiscard]] VkDescriptorSetLayout* get_descriptor_sets_layouts();

  private:
    std::vector<VkDescriptorSetLayout> descriptor_set_layout = {};
    VkPipelineLayout                   pipeline_layout       = VK_NULL_HANDLE;
    VkPipeline                         pipeline              = VK_NULL_HANDLE;
};
