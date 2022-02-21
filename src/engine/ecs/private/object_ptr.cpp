#include "object_ptr.h"
#include "ecs/ecs_new.h"
#include "ecs/family.h"

Component<ComponentBase> ObjectPtr::get_component(Class* component_class)
{
    if (!object_family)
        return {};

    return object_family->get_component(this, component_class);
}

Component<ComponentBase> ObjectPtr::add_component(Class* component_class)
{
    const auto new_signature = object_family->get_signature().added(component_class);

    if (object_family)
        object_family->remove_object(this);

    ecs->find_family(new_signature)->add_object(this);

    return get_component(component_class);
}

void ObjectPtr::remove_component(Class* component_class)
{
    const auto new_signature = object_family->get_signature().erased(component_class);

    if (object_family)
        object_family->remove_object(this);

    ecs->find_family(new_signature)->add_object(this);
}
