#pragma once

#include "ecs/object.h"
#include "ecs_new.h"
#include "family_signature.h"
#include "object_ptr.h"
#include "types/robin_hood_map.h"

#include <cpputils/logger.hpp>

using DataType     = std::byte;
using ObjectHandle = size_t;

template <typename Component_T> class ComponentSmPtr
{
  public:
    ComponentSmPtr() = default;

    Component_T* operator->() const
    {
        return static_cast<Component_T*>(*component);
    }

    operator bool() const
    {
        return component && *component;
    }

  private:
    std::shared_ptr<DataType*> component;
    ObjectHandle               object_handle;
};

class ObjectSmPtr
{
  public:
    template <typename Component_T> ComponentSmPtr<Component_T> get_component()
    {
        return components[Component_T::static_class()];
    }

  private:
    ObjectHandle object_handle;
};

class Family final : NoCopy
{
    class ComponentVector : NoCopy
    {
      public:
        ComponentVector(const Class* type) : size(type->size())
        {
        }

        void move(uint32_t from_index, uint32_t to_index)
        {
            std::memcpy(&data_vector[static_cast<size_t>(to_index) * size], &data_vector[static_cast<size_t>(from_index) * size], size);
        }

        void resize(uint32_t new_count)
        {
            data_vector.resize(static_cast<size_t>(new_count) * size);
        }

      private:
        std::vector<DataType>                   data_vector;
        std::vector<std::shared_ptr<DataType*>> component_map;
        uint32_t                                size;
    };

  public:
    Family(const FamilySignature& in_signature);
    ~Family() override;

    void                                   add_object(ObjectPtr* object);
    void                                   remove_object(ObjectPtr* object);
    [[nodiscard]] Component<ComponentBase> get_component(ObjectPtr* object, Class* component_class);
    [[nodiscard]] const FamilySignature&   get_signature() const
    {
        return signature;
    }

  private:
    void realloc(uint32_t new_object_count)
    {
        object_count = new_object_count;
        for (uint32_t i = 0; i < component_count; ++i)
            components[i].resize(object_count);

        object_map.resize(object_count);
    }
    
    ComponentVector*        components;
    std::vector<ObjectPtr*> object_map;
    const uint32_t          component_count;
    uint32_t                object_count;
    const FamilySignature   signature;
};


/**
 * \brief ///////////////////////////////////////////////////////////////////////////
 */

class A
{
};

class B
{
};

void test()
{
    Ecs_New ecs;
    ComponentSmPtr<A> a;
    ObjectSmPtr obj = ecs.new_object();
    a = obj.get_component<A>();
}