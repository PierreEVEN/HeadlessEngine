
#include "vk_render_pass.h"

#include "resources/vk_resource_descriptors.h"
#include "resources/vk_resource_render_pass.h"

namespace gfx::vulkan
{
RenderPass_VK::RenderPass_VK(const Config& frame_graph_config, bool in_present_pass)
    : RenderPass(frame_graph_config, in_present_pass), descriptor_pool_manager(std::make_unique<DescriptorPoolManager>(frame_graph_config.pass_name))
{
    render_pass = TGpuHandle<RenderPassResource_VK>(stringutils::format("render_pass:%s", frame_graph_config.pass_name.c_str()), RenderPassResource_VK::CreateInfos{
                                                                                                                                     .render_pass_config = frame_graph_config,
                                                                                                                                     .in_present_pass    = in_present_pass,
                                                                                                                                 });

    /*
    if (Device::get().use_bindless_descriptors())
    {
        bindless_descriptor_pool = TGpuHandle<DescriptorPoolResource_VK>("bindless_descriptor_pool", DescriptorPoolResource_VK::CreateInfos{
                                                                                                         .max_descriptor_per_pool = 16536,
                                                                                                         .bindless_pool           = true,
                                                                                                     });

        bindless_descriptor_set_layout = TGpuHandle<DescriptorSetLayoutResource_VK>("bindless_descriptor_pool", DescriptorSetLayoutResource_VK::CreateInfos{
                                                                                                                    .vertex_bindings         = {},
                                                                                                                    .fragment_bindings       = {},
                                                                                                                    .bindless_descriptor_set = true,
                                                                                                                });

        bindless_descriptors = TGpuHandle<DescriptorSetResource_VK>("bindless_descriptor_pool", DescriptorSetResource_VK::CreateInfos{
                                                                                                    .desc_set_layout = bindless_descriptor_set_layout,
                                                                                                    .descriptor_pool = bindless_descriptor_pool,
                                                                                                });
    }
    */
}
} // namespace gfx::vulkan
