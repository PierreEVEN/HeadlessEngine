#include "gfx/gfx.h"


#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_physical_device.h"
#include "vulkan/vk_instance.h"
#include "vulkan/vk_unit.h"
#endif

namespace gfx
{
void init()
{
#if GFX_USE_VULKAN
    vulkan::instance::create();
    fetch_physical_devices();
    select_physical_device(find_best_physical_device(get_physical_devices()));
    Device::create_device<vulkan::Device_VK>(3);
    vulkan::allocator::create();
#else
    static_assert(false, "backend not supported");
#endif
}

void next_frame()
{
}

void destroy()
{
    RenderPass::destroy_passes();
    Device::destroy_device();
#if GFX_USE_VULKAN
    vulkan::allocator::destroy();
    vulkan::instance::destroy();
#endif
}
} // namespace gfx
