#include "gfx/gfx.h"

#include <cpputils/logger.hpp>

#if GFX_USE_VULKAN
#include "vulkan/allocator.h"
#include "vulkan/device.h"
#include "vulkan/vk_physical_device.h"
#include "vulkan/instance.h"
#include "vulkan/unit.h"
#endif

namespace gfx
{
void init()
{
#if GFX_USE_VULKAN
    vulkan::set_image_count(3);
    vulkan::instance::create();
    fetch_physical_devices();
    select_physical_device(find_best_physical_device(get_physical_devices()));
    vulkan::device::create();
    vulkan::allocator::create();
#else
    LOG_ERROR("there is no supported graphic backend.");
#endif
}

void next_frame()
{
#if GFX_USE_VULKAN
    vulkan::next_frame();
#endif
}

void destroy()
{
#if GFX_USE_VULKAN
    vulkan::allocator::destroy();
    vulkan::device::destroy();
    vulkan::instance::destroy();
#endif
}
} // namespace gfx
