#pragma once
#include "vk_descriptor_pool.h"
#include "gfx/master_material.h"

#include "vk_unit.h"
#include "resources/vk_resource_pipeline.h"

#include <vulkan/vulkan.h>

namespace gfx::vulkan
{

class MasterMaterial_VK final : public MasterMaterial
{
  public:
    MasterMaterial_VK(const std::string& name, const MaterialOptions& options) : MasterMaterial(name, options)
    {
    }
    ~MasterMaterial_VK() override;

    struct MaterialPassData
    {
        TGpuHandle<DescriptorSetLayoutResource_VK> descriptor_set_layout = {};
        TGpuHandle<PipelineResource_VK>            pipeline              = {};
        TGpuHandle<PipelineLayoutResource_VK>      pipeline_layout       = {};
        TGpuHandle<ShaderModuleResource_VK>        vertex_module         = {};
        TGpuHandle<ShaderModuleResource_VK>        fragment_module       = {};
    };

    void rebuild_material(const shader_builder::CompilationResult& compilation_results) override;

    [[nodiscard]] TGpuHandle<PipelineLayoutResource_VK> get_pipeline_layout(const RenderPassID& render_pass) const
    {
        if (!render_pass)
            return {};
        return per_pass_data[render_pass].pipeline_layout;
    }

    [[nodiscard]] TGpuHandle<PipelineResource_VK> get_pipeline(const RenderPassID& render_pass) const
    {
        if (!render_pass)
            return {};
        return per_pass_data[render_pass].pipeline;
    }

    [[nodiscard]] const TGpuHandle<DescriptorSetLayoutResource_VK>& get_descriptor_set_layouts(const RenderPassID& render_pass) const
    {
        return per_pass_data[render_pass].descriptor_set_layout;
    }

    static VkDescriptorType vk_descriptor_type(shader_builder::EBindingType type)
    {
        switch (type)
        {
        case shader_builder::EBindingType::SAMPLER:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case shader_builder::EBindingType::COMBINED_IMAGE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case shader_builder::EBindingType::SAMPLED_IMAGE:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case shader_builder::EBindingType::STORAGE_IMAGE:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case shader_builder::EBindingType::UNIFORM_TEXEL_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case shader_builder::EBindingType::STORAGE_TEXEL_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case shader_builder::EBindingType::UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case shader_builder::EBindingType::STORAGE_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case shader_builder::EBindingType::UNIFORM_BUFFER_DYNAMIC:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case shader_builder::EBindingType::STORAGE_BUFFER_DYNAMIC:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case shader_builder::EBindingType::INPUT_ATTACHMENT:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:;
            LOG_FATAL("unhandled case");
        }
    }

  private:
    void create_modules(const shader_builder::CompilationResult& compilation_results);

    void                             clear();
    RenderPassData<MaterialPassData> per_pass_data;
};
} // namespace gfx::vulkan
