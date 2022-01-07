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
        SwapchainImageResource<VkDescriptorSetLayout> descriptor_set_layout;
        VkPipeline                                    pipeline        = VK_NULL_HANDLE;
        VkPipelineLayout                              layout          = VK_NULL_HANDLE;
        VkShaderModule                                vertex_module   = VK_NULL_HANDLE;
        VkShaderModule                                fragment_module = VK_NULL_HANDLE;
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

    const SwapchainImageResource<VkDescriptorSetLayout>& get_descriptor_set_layout(const RenderPassID& render_pass)&
    {
        return per_pass_data[render_pass].descriptor_set_layout;
    }

  private:
    void create_modules(const shader_builder::CompilationResult& compilation_results);

    void                             clear();
    RenderPassData<MaterialPassData> per_pass_data;
};
} // namespace gfx::vulkan
