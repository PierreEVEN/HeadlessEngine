#pragma once
#include "gfx/resource/gpu_resource.h"
#include "vulkan/resources/vk_resource_descriptors.h"
#include "vulkan/vk_render_pass.h"

#include <vulkan/vulkan.h>

namespace gfx::vulkan
{

class ShaderModuleResource_VK final
{
  public:
    struct CreateInfos
    {
        const std::vector<uint32_t>& spirv_code;
    };

    ShaderModuleResource_VK(const std::string& name, const CreateInfos& create_infos);
    ~ShaderModuleResource_VK();

    VkShaderModule shader_module;
};

class PipelineLayoutResource_VK final
{
  public:
    struct CreateInfos
    {
        const uint32_t                                   vertex_push_constant_size;
        const uint32_t                                   fragment_push_constant_size;
        const TGpuHandle<DescriptorSetLayoutResource_VK> descriptor_set_layout;
    };

    PipelineLayoutResource_VK(const std::string& name, const CreateInfos& create_infos);
    ~PipelineLayoutResource_VK();

    VkPipelineLayout pipeline_layout;

  private:
    const TGpuHandle<DescriptorSetLayoutResource_VK> descriptor_set_layout;
};

class PipelineResource_VK final
{
  public:
    struct CreateInfos
    {
        const TGpuHandle<RenderPassResource_VK>      render_pass;
        const TGpuHandle<PipelineLayoutResource_VK>  pipeline_layout;
        const TGpuHandle<ShaderModuleResource_VK>    vertex_stage;
        const TGpuHandle<ShaderModuleResource_VK>    fragment_stage;
        const shader_builder::ShaderProperties&      shader_properties;
        const std::vector<shader_builder::Property>& vertex_inputs;
    };

    PipelineResource_VK(const std::string& name, const CreateInfos& create_infos);
    ~PipelineResource_VK();

    VkPipeline pipeline;

  private:
    const TGpuHandle<ShaderModuleResource_VK>   vertex_stage;
    const TGpuHandle<ShaderModuleResource_VK>   fragment_stage;
    const TGpuHandle<PipelineLayoutResource_VK> pipeline_layout;
    const TGpuHandle<RenderPassResource_VK>     render_pass;
};
} // namespace gfx::vulkan