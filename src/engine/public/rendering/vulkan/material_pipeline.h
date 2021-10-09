#pragma once

#include "assets/asset_ptr.h"
#include "rendering/mesh/vertex.h"

#include <optional>
#include <vulkan/vulkan.h>

class AShader;

struct PipelineInfos
{
    // Pipeline config
    VkBool32            depth_test            = VK_TRUE;
    VkBool32            wireframe             = VK_FALSE;
    float               wireframe_lines_width = 1;
    VkPrimitiveTopology topology              = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode       polygon_mode          = VK_POLYGON_MODE_FILL;
};

class MaterialPipeline final
{
  public:
    MaterialPipeline(const PipelineInfos& pipeline_infos, const std::string& render_pass, const std::vector<VkDescriptorSetLayoutBinding>& layout_bindings, const std::vector<TAssetPtr<AShader>>& stages, const std::optional<PushConstant>& push_constants);
    ~MaterialPipeline();

    [[nodiscard]] VkPipelineLayout*      get_pipeline_layout();
    [[nodiscard]] VkPipeline             get_pipeline() const;
    [[nodiscard]] VkDescriptorSetLayout* get_descriptor_sets_layouts();

  private:
    std::vector<VkDescriptorSetLayout> descriptor_set_layout = {};
    VkPipelineLayout                   pipeline_layout       = VK_NULL_HANDLE;
    VkPipeline                         pipeline              = VK_NULL_HANDLE;
};
