#include "unit.h"

namespace gfx::vulkan
{

uint8_t image_index        = 0;
uint8_t render_image_count = 1;

void set_image_count(uint8_t image_count)
{
    render_image_count = image_count;
}

uint8_t get_image_count()
{
    return render_image_count;
}

void set_frame(uint8_t new_image_index)
{
    image_index = new_image_index;
}

uint8_t get_image_index()
{
    return image_index;
}

} // namespace gfx::vulkan