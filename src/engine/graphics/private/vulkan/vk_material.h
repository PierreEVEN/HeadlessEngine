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

    [[nodiscard]] const VkPipelineLayout* get_pipeline_layout(const RenderPassID& render_pass) const
    {
        if (!render_pass)
            return nullptr;
        return &per_pass_data[render_pass].layout;
    }

    [[nodiscard]] const VkPipeline* get_pipeline(const RenderPassID& render_pass) const
    {
        if (!render_pass)
            return nullptr;
        return &per_pass_data[render_pass].pipeline;
    }

  private:
    void create_modules(const shader_builder::CompilationResult& compilation_results);

    void                             clear();
    RenderPassData<MaterialPassData> per_pass_data;
};
} // namespace gfx::vulkan
