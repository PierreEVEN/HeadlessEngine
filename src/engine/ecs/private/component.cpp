

#include "ecs/component.h"

#include "ecs/actor.h"

namespace ecs
{
void IComponent::resize_component_memory(size_t new_size, ActorVariant* variant, size_t component_type_index) const
{
    if (new_size > variant->per_component_data_size[component_type_index])
    {
        const size_t element_size = type_size();

        variant->per_component_data_size[component_type_index] *= 2;
        variant->per_component_data_size[component_type_index] += element_size;

        // Move components to their new memory block
        auto* new_component_data = new ComponentDataType[variant->per_component_data_size[component_type_index]];
        for (std::size_t e = 0; e < variant->linked_actors.size(); ++e)
        {
            component_move(&variant->component_data[component_type_index][e * element_size], &new_component_data[e * element_size]);
            component_destroy(&variant->component_data[component_type_index][e * element_size]);
        }
        delete[] variant->component_data[component_type_index];

        variant->component_data[component_type_index] = new_component_data;
    }
}
} // namespace ecs