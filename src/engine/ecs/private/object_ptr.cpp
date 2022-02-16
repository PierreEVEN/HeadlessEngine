#include <object_ptr.h>

#include <family.h>

Component<ComponentPtr> ObjectPtr::get_component(Class* component_class)
{
    if (!object_family)
        return {};

    return object_family->get_component(this, component_class);
}

Component<ComponentPtr> ObjectPtr::add_component(Class* component_class)
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
