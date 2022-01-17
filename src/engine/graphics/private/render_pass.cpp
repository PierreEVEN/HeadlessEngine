
#include "gfx/render_pass.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/vk_render_pass.h"
#endif

namespace gfx
{

static RenderPassData<RenderPass*> render_passes;

RenderPass::RenderPass(const Config& frame_graph_config, bool in_present_pass) : config(frame_graph_config), render_pass_id(RenderPassID::declare(frame_graph_config.pass_name)), present_pass(in_present_pass)
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
    for (const auto& pass : render_passes)
        delete pass;
    render_passes.clear();
}

RenderPass* RenderPass::declare_internal(const Config& frame_graph_config, bool present_pass)
{
#if GFX_USE_VULKAN
    const auto render_pass    = new vulkan::RenderPass_VK(frame_graph_config, present_pass);
    const auto   pass_id  = RenderPassID::get(frame_graph_config.pass_name);
    RenderPass*& pass_ptr = render_passes.init(pass_id);
    pass_ptr              = render_pass;
    return pass_ptr;
#else
    static_assert(false, "backend not supported");
#endif
}

} // namespace gfx
