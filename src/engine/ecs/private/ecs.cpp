#include "ecs/ecs.h"

namespace ecs
{

void register_component_type_internal(const ComponentData& component_data)
{
    (void)component_data;
}

void MyComponent::pre_render()
{
}

void MyComponent::render(gfx::CommandBuffer* command_buffer)
{
    internal_val++;
}
} // namespace ecs