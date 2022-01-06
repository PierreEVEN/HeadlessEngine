
#include "gfx/render_pass.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/vk_render_pass.h"
#endif

namespace gfx
{

static std::unordered_map<std::string, std::unique_ptr<RenderPass>> render_passes;

RenderPass::RenderPass(const Config& frame_graph_config) : config(frame_graph_config)
{
}

RenderPass* RenderPass::find(const std::string& render_pass_name)
{
    const auto& it = render_passes.find(render_pass_name);
    if (it != render_passes.end())
        return it->second.get();
    return nullptr;
}

void RenderPass::destroy_passes()
{
    render_passes.clear();
}

RenderPass* RenderPass::declare(const Config& frame_graph_config, bool present_pass)
{
#if GFX_USE_VULKAN
    if (render_passes.contains(frame_graph_config.pass_name))
        LOG_FATAL("a render pass named %s already exists", frame_graph_config.pass_name.c_str());
    render_passes[frame_graph_config.pass_name] = std::make_unique<vulkan::RenderPass_VK>(frame_graph_config);
    if (present_pass)
        render_passes[frame_graph_config.pass_name]->present_pass = true;
    return render_passes[frame_graph_config.pass_name].get();
#endif
    return nullptr;
}

} // namespace gfx
