
#include "vk_render_pass.h"

#include "resources/vk_resource_render_pass.h"

namespace gfx::vulkan
{
RenderPass_VK::RenderPass_VK(const Config& frame_graph_config, bool in_present_pass)
    : RenderPass(frame_graph_config, in_present_pass), descriptor_pool_manager(std::make_unique<DescriptorPoolManager>(frame_graph_config.pass_name))
{
    render_pass = TGpuHandle<RenderPassResource_VK>(stringutils::format("renderpass:%s", frame_graph_config.pass_name.c_str()), RenderPassResource_VK::CreateInfos{
                                                                                                                            .render_pass_config = frame_graph_config,
                                                                                                                            .in_present_pass    = in_present_pass,
                                                                                                                        });
}
} // namespace gfx::vulkan
