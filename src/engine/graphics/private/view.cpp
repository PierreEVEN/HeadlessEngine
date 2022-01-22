#include "gfx/view.h"

#include "gfx/buffer.h"

namespace gfx
{
View::View()
{
    camera_buffer = gfx::Buffer::create("camera_ubo", 1, sizeof(CameraBufferData), gfx::EBufferUsage::UNIFORM_BUFFER, gfx::EBufferAccess::CPU_TO_GPU, gfx::EBufferType::IMMEDIATE);
}

const std::shared_ptr<Buffer>& View::get_buffer()
{
    if (dirty)
    {
        camera_buffer->set_data(
            [&](void* data)
            {
                memcpy(data, &buffer_data, sizeof(CameraBufferData));
            });
        dirty = false;
    }

    return camera_buffer;
}
}
