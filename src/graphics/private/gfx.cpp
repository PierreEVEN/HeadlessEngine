#include "gfx/gfx.h"

#include "vulkan/allocator.h"
#include "vulkan/device.h"
#include "vulkan/hardware.h"
#include "vulkan/instance.h"

namespace gfx
{
void init()
{
    vulkan::instance::create();
    vulkan::hardware::select_hardware();
    vulkan::device::create();
    vulkan::allocator::create();
}

void destroy()
{
    vulkan::allocator::destroy();
    vulkan::device::destroy();
    vulkan::instance::destroy();
}
} // namespace gfx
