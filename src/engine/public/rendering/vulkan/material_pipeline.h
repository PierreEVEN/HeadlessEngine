#pragma once

#include "rendering/renderer/render_pass_description.h"
#include "shader_module.h"

#include <vector>
#include <vulkan/vulkan.h>

struct MaterialPipelineBindings
{
    std::vector<VkDescriptorSetLayoutBinding> descriptor_bindings  = {};
    std::unordered_map<std::string, uint32_t> vertex_binding_map   = {};
    std::unordered_map<std::string, uint32_t> fragment_binding_map = {};
};

struct MaterialPipelineConfiguration
{
    TAssetPtr<AShader>       vertex_module         = {};
    TAssetPtr<AShader>       fragment_module       = {};
    std::string              renderer_stages       = {};
    MaterialPipelineBindings descriptor_bindings   = {};
    VkBool32                 depth_test            = VK_TRUE;
    VkBool32                 wireframe             = VK_FALSE;
    float                    wireframe_lines_width = 1;
    VkPrimitiveTopology      topology              = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode            polygon_mode          = VK_POLYGON_MODE_FILL;

    [[nodiscard]] bool is_valid() const
    {
        return vertex_module && fragment_module && !renderer_stages.empty();
    }
};

class MaterialPipeline final
{
  public:
    MaterialPipeline() = default;
    ~MaterialPipeline();

    void update_configuration(const MaterialPipelineConfiguration& in_configuration);
    void init_or_rebuild_pipeline();

    [[nodiscard]] VkPipelineLayout                     get_pipeline_layout() const;
    [[nodiscard]] VkPipeline                           get_pipeline() const;
    [[nodiscard]] const std::vector<VkDescriptorSet>&  get_descriptor_sets() const;
    [[nodiscard]] const MaterialPipelineConfiguration& get_pipeline_configuration() const;

  private:
    void destroy();
    void create_pipeline();

    MaterialPipelineConfiguration pipeline_configuration = {};
    std::vector<VkDescriptorSet>  descriptor_sets        = {};
    VkDescriptorSetLayout         descriptor_set_layout  = VK_NULL_HANDLE;
    VkPipelineLayout              pipeline_layout        = VK_NULL_HANDLE;
    VkPipeline                    pipeline               = VK_NULL_HANDLE;
    bool                          is_dirty               = true;
};
