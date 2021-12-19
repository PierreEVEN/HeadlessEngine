#include "ecs/ecs.h"

namespace ecs
{

void MyComponent::pre_render()
{
}

void MyComponent::render(gfx::CommandBuffer* command_buffer)
{
    (void)command_buffer;
    internal_val++;
}
} // namespace ecs