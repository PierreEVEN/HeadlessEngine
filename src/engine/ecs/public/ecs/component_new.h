#pragma once

class Object;
class ComponentPtr;

// Smart containers
template <typename Component_T> class Component
{
  public:
    Component()
    {
    }

    Component_T* operator->()
    {
        return owner ? owner->get_component<Component_T>() : nullptr;
    }

    operator bool() const
    {
        return owner;
    }

    [[nodiscard]] Object& object() const
    {
        return *owner;
    }

  private:
    Object owner;
};
