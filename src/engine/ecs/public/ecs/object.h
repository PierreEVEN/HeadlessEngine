#pragma once
#include "reflection/class.h"
#include "reflection/object_base.h"
#include "types/no_copy.h"

class ComponentBase;
class Object;
class Ecs_New;
class Class;
class ObjectPtr;

template <typename Component_T>
concept is_defined = (std::is_object_v<Component_T> && !std::is_pointer_v<Component_T> && (sizeof(Component_T) > 0));

template <typename Component_T>
concept ComponentType = (!is_defined<Component_T> || std ::is_base_of_v<ComponentBase, Component_T>);

// Smart containers
template <ComponentType Component_T> class Component
{
  public:
    Component() : component(nullptr)
    {
    }

    template <typename Other_T> Component(const Component<Other_T>& other)
    {
        if (!other)
        {
            component = nullptr;
            return;
        }

        component = cast<Component_T>(other.component);
    }

    Component_T* operator->();

    operator bool() const
    {
        return component;
    }

    [[nodiscard]] Object object() const;

  private:
    //@TODO CA MARCHE PAS CA CACA
    Component_T* component = nullptr;
};

class ComponentBase : public ObjectBase, NoCopy
{
  public:
    template <ComponentType Component_T> [[nodiscard]] Component<Component_T> get_component();

    [[nodiscard]] Object& object() const
    {
        return *owner;
    }

    virtual void on_move(Ecs_New*)
    {
    }

    virtual void tick()
    {
    }

    virtual void pre_render()
    {
    }

  private:
    Object* owner = nullptr;
};

class Object final
{
    template <ComponentType Component_T> friend class Component;

  public:
    Object() : object_ptr(nullptr)
    {
    }

    Object(Ecs_New* source_ecs);

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
    Component<ComponentBase> get_component(Class* component_class) const;
    Component<ComponentBase> add_component(Class* component_class) const;
    void                     remove_component(Class* component_class) const;

    operator bool() const
    {
        return object_ptr;
    }

    void move_to(Ecs_New* new_parent) const;

  private:
    Object(ObjectPtr* other) : object_ptr(other)
    {
    }

    void decrement_ref();
    void increment_ref() const;

    ObjectPtr* object_ptr;
};

template <ComponentType Component_T> Component_T* Component<Component_T>::operator->()
{
    return component;
}

template <ComponentType Component_T> Object Component<Component_T>::object() const
{
    return component->object();
}

template <ComponentType Component_T> Component<Component_T> ComponentBase::get_component()
{
    return owner->get_component<Component_T>();
}
