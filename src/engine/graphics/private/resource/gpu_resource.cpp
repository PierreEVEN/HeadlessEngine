#include "gfx/resource/gpu_resource.h"

#include "gfx/resource/device.h"

namespace gfx
{
void IGpuHandle::destroy()
{
    Device::get().delete_resource(resource);
    resource = nullptr;
}
} // namespace gfx
