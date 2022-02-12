#include "vulkan/vk_master_material.h"

#include "vk_errors.h"
#include "vk_render_pass.h"
#include "vulkan/vk_device.h"

namespace gfx::vulkan
{
MasterMaterial_VK::~MasterMaterial_VK()
{
    clear();
}

void MasterMaterial_VK::create_modules(const shader_builder::CompilationResult& compilation_results)
{
    for (auto& pass : compilation_results.passes)
    {
        if (!RenderPassID::exists(pass.first))
            continue;

        MaterialPassData& pass_data = per_pass_data.init(RenderPassID::get(pass.first));
        pass_data.vertex_module     = TGpuHandle<ShaderModuleResource_VK>(stringutils::format("vertex_stage:material=%s", material_name.c_str()), ShaderModuleResource_VK::CreateInfos{
                                                                                            .spirv_code = pass.second.vertex.spirv,
                                                                                        });
        pass_data.fragment_module   = TGpuHandle<ShaderModuleResource_VK>(stringutils::format("fragment_stage:material=%s", material_name.c_str()), ShaderModuleResource_VK::CreateInfos{
                                                                                                .spirv_code = pass.second.fragment.spirv,
                                                                                            });
    }
}

void MasterMaterial_VK::clear()
{
    per_pass_data.clear();
}

void MasterMaterial_VK::rebuild_material(const shader_builder::CompilationResult& compilation_results)
{
    MasterMaterial::rebuild_material(compilation_results);
    clear();

    create_modules(compilation_results);

    auto pass_data = per_pass_data.begin();
    for (; pass_data != per_pass_data.end(); ++pass_data)
    {
        std::vector<VkDescriptorSetLayoutBinding> pipeline_bindings = {};

        const auto vertex_reflection_data   = get_vertex_reflection(pass_data.id());
        const auto fragment_reflection_data = get_fragment_reflection(pass_data.id());

        pass_data->descriptor_set_layout = TGpuHandle<DescriptorSetLayoutResource_VK>(stringutils::format("desc_set_layout:mat=%s:pass=%s", material_name.c_str(), pass_data.id().name().c_str()),
                                                                                      DescriptorSetLayoutResource_VK::CreateInfos{
                                                                                          .vertex_bindings   = vertex_reflection_data.bindings,
                                                                                          .fragment_bindings = fragment_reflection_data.bindings,
                                                                                      });
        pass_data->pipeline_layout       = TGpuHandle<PipelineLayoutResource_VK>(stringutils::format("pipeline_layout:mat=%s:pass=%s", material_name.c_str(), pass_data.id().name().c_str()),
                                                                           PipelineLayoutResource_VK::CreateInfos{
                                                                               .vertex_push_constant_size   = vertex_reflection_data.push_constant ? vertex_reflection_data.push_constant->structure_size : 0,
                                                                               .fragment_push_constant_size = fragment_reflection_data.push_constant ? fragment_reflection_data.push_constant->structure_size : 0,
                                                                               .descriptor_set_layout       = pass_data->descriptor_set_layout,
                                                                           });
        pass_data->pipeline              = TGpuHandle<PipelineResource_VK>(stringutils::format("pipeline:mat=%s:pass=%s", material_name.c_str(), pass_data.id().name().c_str()),
                                                              PipelineResource_VK::CreateInfos{
                                                                  .render_pass       = dynamic_cast<RenderPass_VK*>(RenderPass::find(pass_data.id()))->get(),
                                                                  .pipeline_layout   = pass_data->pipeline_layout,
                                                                  .vertex_stage      = pass_data->vertex_module,
                                                                  .fragment_stage    = pass_data->fragment_module,
                                                                  .shader_properties = compilation_results.properties,
                                                                  .vertex_inputs     = material_options.input_stage_override ? material_options.input_stage_override.value() : get_vertex_reflection(pass_data.id()).inputs,
                                                              });
    }
}
} // namespace gfx::vulkan
