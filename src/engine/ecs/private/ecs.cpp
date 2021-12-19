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

std::unique_ptr<ECS> ecs_singleton;

ECS& ECS::get()
{
    if (!ecs_singleton)
        ecs_singleton = std::unique_ptr<ECS>(new ECS());
    return *ecs_singleton.get();
}

struct MyComp
{
    MyComp(float val)
    {
        (void)val;
    }

    void tick()
    {
    }
};
struct MyComp2
{
    MyComp2(float val)
    {
        (void)val;
    }

    void pre_render()
    {
    }
};

void ecs_test()
{
    /*
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
    */

    auto ecs = ECS::get();

    ecs.register_component_type<MyComp>();
    ecs.register_component_type<MyComp2>();

    ActorID actor_1 = ecs.make_new_actor_id();
    ecs.add_empty_actor(actor_1);
    ecs.add_component<MyComp>(actor_1, 10.f);
    ecs.add_component<MyComp2>(actor_1, 10.f);
}

void IComponent::resize_component_memory(size_t new_size, ActorVariant* variant, size_t component_type_index) const
{
    if (new_size > variant->per_component_data_size[0])
    {
        size_t size = type_size();

        LOG_INFO("need to resize data from %ld to %ld", variant->per_component_data_size[0], new_size);

        variant->per_component_data_size[0] *= 2;
        variant->per_component_data_size[0] += size;

        // Move components to their new memory block
        auto* new_component_data = new ComponentDataType[variant->per_component_data_size[component_type_index]];
        for (std::size_t e = 0; e < variant->linked_actors.size(); ++e)
        {
            component_move(&variant->component_data[component_type_index][e * size], &new_component_data[e * size]);
            component_destroy(&variant->component_data[component_type_index][e * size]);
        }
        delete[] variant->component_data[0];

        variant->component_data[0] = new_component_data;
    }
}
} // namespace ecs