
#include "gfx/render_pass.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/vk_render_pass.h"
#endif

namespace gfx
{

static RenderPassData<RenderPass*> render_passes;

RenderPass::RenderPass(const Config& frame_graph_config) : config(frame_graph_config), render_pass_id(RenderPassID::declare(frame_graph_config.pass_name))
{
}

RenderPass* RenderPass::find(const RenderPassID& render_pass_name)
{
    if (render_pass_name)
        return render_passes[render_pass_name];
    LOG_FATAL("failed to find render pass");
}

void RenderPass::destroy_passes()
{
}

RenderPass* RenderPass::declare(const Config& frame_graph_config, bool present_pass)
{
#if GFX_USE_VULKAN
    const auto pass_id     = RenderPassID::declare(frame_graph_config.pass_name);
    render_passes[pass_id] = new vulkan::RenderPass_VK(frame_graph_config);
    return render_passes[pass_id];
#endif
    return nullptr;
}

} // namespace gfx
