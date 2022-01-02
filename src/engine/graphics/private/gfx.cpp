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
