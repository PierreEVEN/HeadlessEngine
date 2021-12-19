#pragma once
#include "component_meta_class.h"

#include <cpputils/logger.hpp>

namespace ecs
{
class MyComponent
{
  public:
    void pre_render();
    void render(gfx::CommandBuffer* command_buffer);
    int  internal_val;
};

inline void ecs_test()
{
    // register different components
    auto comp_data = register_component<MyComponent>();

    // Create an example of components
    const size_t component_count = 10000000;
    auto*        component_data  = static_cast<uint8_t*>(malloc(sizeof(MyComponent) * 10000000));
    for (size_t i = 0; i < component_count; ++i)
        new (component_data + i * sizeof(MyComponent)) MyComponent();

    // Example game loop
    do
    {
        // foreach comp class
        do
        {
            if (comp_data.pre_render_runner)
                comp_data.pre_render_runner->execute(component_data, component_count);

            if (comp_data.render_runner)
                comp_data.render_runner->execute(component_data, component_count, nullptr);

            if (comp_data.tick_runner)
                comp_data.tick_runner->execute(component_data, component_count);
        } while (false);
    } while (false);

    free(component_data);
}

} // namespace ecs