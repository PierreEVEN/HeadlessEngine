#include "gfx/resource/gpu_resource.h"

#include "gfx/resource/device.h"

namespace gfx
{
void IGpuHandle::destroy()
{
    auto* to_delete = resource_ptr;
    resource_ptr    = nullptr;
    Device::get().delete_resource(to_delete);
}
} // namespace gfx
