#pragma once

#include "ecs/component_new.h"
#include "reflection/class.h"
#include <cpputils/logger.hpp>

class Class;
class ObjectPtr;

using DataType = uint8_t;

struct FamilySignature
{
    friend class Family;

  public:
    FamilySignature(std::vector<Class*> in_elements) : elements(std::move(in_elements))
    {
        std::ranges::sort(elements);
    }

    FamilySignature erased(Class* removed_component) const
    {
        auto signature = elements;
        signature.erase(std::ranges::find(signature, removed_component));
        return signature;
    }

    FamilySignature added(Class* added_component) const
    {
        auto signature = elements;
        signature.emplace_back(added_component);
        return signature;
    }

  private:
    std::vector<Class*> elements;
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
        std::vector<DataType> data_vector;
        uint32_t              size;
    };

  public:
    Family(const FamilySignature& in_signature);
    ~Family() override;

    void                                  add_object(ObjectPtr* object);
    void                                  remove_object(ObjectPtr* object);
    [[nodiscard]] Component<ComponentPtr> get_component(ObjectPtr* object, Class* component_class);
    [[nodiscard]] const FamilySignature&  get_signature() const
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
