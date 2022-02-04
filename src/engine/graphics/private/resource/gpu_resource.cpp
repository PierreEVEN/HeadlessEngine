#include "gfx/resource/gpu_resource.h"

namespace gfx
{
void IGpuHandle::destroy()
{
    TestDevice::delete_resource(resource);
    resource = nullptr;
}
} // namespace gfx
