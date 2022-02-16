

#include "ecs/object.h"

#include "object_ptr.h"

Component<ComponentPtr> Object::get_component(Class* component_class) const
{
    return object_ptr->get_component(component_class);
}

Component<ComponentPtr> Object::add_component(Class* component_class) const
{
    return object_ptr->add_component(component_class);
}

void Object::remove_component(Class* component_class) const
{
    object_ptr->remove_component(component_class);
}

void Object::move_to(Ecs_New* new_parent) const
{
    object_ptr->move_to(new_parent);
}

void Object::decrement_ref()
{
    if (!object_ptr)
        return;

    object_ptr->ref_counter--;
    if (object_ptr->ref_counter == 0)
        delete object_ptr;

    object_ptr = nullptr;
}

void Object::increment_ref() const
{
    if (!object_ptr)
        return;

    object_ptr->ref_counter++;
}
