#pragma once
#include "component_new.h"
#include "object_ptr.h"

class Ecs_New;
class Class;
class ObjectPtr;

class Object final
{
  public:
    Object() : object_ptr(nullptr)
    {
    }

    Object(Ecs_New* source_ecs)
        : object_ptr(new ObjectPtr(source_ecs))
    {
    }

    Object(const Object& other) : object_ptr(other.object_ptr)
    {
        increment_ref();
    }

    Object(Object&& other) noexcept : object_ptr(other.object_ptr)
    {
        increment_ref();
    }

    Object& operator=(const Object& other)
    {
        if (&other == this)
            return *this;
        decrement_ref();
        object_ptr = other.object_ptr;
        increment_ref();
        return *this;
    }

    Object& operator=(Object&& other) noexcept
    {
        if (&other == this)
            return *this;
        decrement_ref();
        object_ptr = other.object_ptr;
        increment_ref();
        return *this;
    }

    ~Object()
    {
        decrement_ref();
    }

    template <typename Component_T> Component<Component_T> get_component()
    {
        return get_component(Component_T::static_class());
    }
    template <typename Component_T, typename... Args_T> Component<Component_T> add_component()
    {
        return add_component(Component_T::static_class());
    }
    template <typename Component_T> void remove_component()
    {
        remove_component(Component_T::static_class());
    }
    Component<ComponentPtr> get_component(Class* component_class) const;
    Component<ComponentPtr> add_component(Class* component_class) const;
    void                    remove_component(Class* component_class) const;

    operator bool() const
    {
        return object_ptr;
    }

    void move_to(Ecs_New* new_parent) const;

  private:
    void decrement_ref();
    void increment_ref() const;

    ObjectPtr* object_ptr;
};