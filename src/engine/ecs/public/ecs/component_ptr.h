#pragma once
#include "ecs/component_new.h"
#include "reflection/class.h"
#include "types/no_copy.h"

class Ecs_New;

class ComponentPtr : public NoCopy
{
  public:
    template <typename Component_T> [[nodiscard]] Component<Component_T> get_component()
    {
        return owner->get_component<Component_T>();
    }

    [[nodiscard]] Object& object() const
    {
        return *owner;
    }

    virtual void on_move(Ecs_New* new_parent)
    {
    }

    virtual void tick()
    {
    }

    virtual void pre_render()
    {
    }

    [[nodiscard]] virtual Class* get_class() const = 0;

  private:
    Object* owner;
};
