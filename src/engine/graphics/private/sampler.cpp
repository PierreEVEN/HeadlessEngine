#include "gfx/sampler.h"

#if GFX_USE_VULKAN
#include "vulkan/vk_sampler.h"
#endif

std::shared_ptr<gfx::Sampler> gfx::Sampler::create(const std::string& sampler_name, const SamplerOptions& options)
{
#if GFX_USE_VULKAN
    return std::make_shared<vulkan::Sampler_VK>(sampler_name, options);
#else
    static_assert(false, "backend not supported");
#endif
}
