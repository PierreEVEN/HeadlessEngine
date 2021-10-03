#pragma once

#include "shader_module.h"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

class MaterialPipeline final
{
  public:
    ~MaterialPipeline();

    void set_vertex_module(std::shared_ptr<ShaderModule> in_module);
    void set_fragment_module(std::shared_ptr<ShaderModule> in_module);
    void enable_depth_test(bool b_in_depth_test);
    void set_wireframe(bool b_in_wireframe);
    void set_wireframe_width(float in_width);
    void set_push_constant_ranges(std::shared_ptr<VkPushConstantRange> in_push_constants);
    void set_topology(VkPrimitiveTopology in_topology);
    void set_polygon_mode(VkPolygonMode in_mode);
    void set_layout_bindings(const std::vector<VkDescriptorSetLayoutBinding>& in_bindings);

    [[nodiscard]] VkPipelineLayout get_pipeline_layout() const
    {
        return pipeline_layout;
    }
    [[nodiscard]] VkPipeline get_pipeline() const
    {
        return pipeline;
    }
    [[nodiscard]] const std::vector<VkDescriptorSet>& get_descriptor_sets() const
    {
        return descriptor_sets;
    }

    void rebuild();

  private:
    void destroy();
    void create_descriptor_sets(std::vector<VkDescriptorSetLayoutBinding> layoutBindings);
    void create_pipeline_layout();
    void create_pipeline();

    void mark_dirty();

    VkDescriptorSetLayout        descriptor_set_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptor_sets       = {};
    VkPipelineLayout             pipeline_layout       = VK_NULL_HANDLE;
    VkPipeline                   pipeline              = VK_NULL_HANDLE;

    std::shared_ptr<ShaderModule>             vertex_module         = nullptr;
    std::shared_ptr<ShaderModule>             fragment_module       = nullptr;
    bool                                      depth_test            = true;
    bool                                      wireframe             = false;
    float                                     wireframe_lines_width = 1;
    std::shared_ptr<VkPushConstantRange>      push_constant_range   = nullptr;
    VkPrimitiveTopology                       topology              = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode                             polygon_mode          = VK_POLYGON_MODE_FILL;
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings       = {};

    bool should_recreate = true;
};
