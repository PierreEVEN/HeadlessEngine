#pragma once
#include "gfx/materials/master_material.h"

#include "vk_unit.h"
#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{

class MasterMaterial_VK final : public MasterMaterial
{
  public:
    MasterMaterial_VK() = default;
    ~MasterMaterial_VK() override;

    struct MaterialPassData
    {
        VkPipeline                                    pipeline        = VK_NULL_HANDLE;
        VkPipelineLayout                              layout          = VK_NULL_HANDLE;
        VkShaderModule                                vertex_module   = VK_NULL_HANDLE;
        VkShaderModule                                fragment_module = VK_NULL_HANDLE;
        SwapchainImageResource<VkDescriptorSetLayout> descriptor_set_layout;
    };

    void rebuild_material(const shader_builder::CompilationResult& compilation_results) override;

    const VkPipelineLayout* get_pipeline_layout(const std::string& render_pass) const
    {
        const auto it = per_pass_data.find(render_pass);
        if (it == per_pass_data.end())
            return nullptr;
        return &it->second.layout;
    }

    const VkPipeline* get_pipeline(const std::string& render_pass) const
    {
        const auto it = per_pass_data.find(render_pass);
        if (it == per_pass_data.end())
            return nullptr;
        return &it->second.pipeline;
    }

  private:
    void create_modules(const shader_builder::CompilationResult& compilation_results);

    void                                              clear();
    std::unordered_map<std::string, MaterialPassData> per_pass_data;
};
} // namespace gfx::vulkan
