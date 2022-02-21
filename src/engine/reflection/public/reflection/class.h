#pragma once
#include "object_base.h"

#include <vcruntime_typeinfo.h>
#include <vector>
#include <cpputils/logger.hpp>

#include <types/no_copy.h>

#define Transient

#define PROPERTY(...)

#define COMPONENT_BODY                                      \
  public:                                                   \
    [[nodiscard]] virtual Class* get_class() const override \
    {                                                       \
        return nullptr;                                     \
    }                                                       \
    static Class* static_class()                            \
    {                                                       \
        return nullptr;                                     \
    }                                                       \
                                                            \
  private:

class Class final : NoCopy
{
  public:
    template <typename Class_T> Class()
    {
        type_size  = sizeof(Class_T);
        class_name = typeid(Class_T).name();
    }

    [[nodiscard]] const char* name() const
    {
        return class_name;
    }

    [[nodiscard]] uint32_t size() const
    {
        return type_size;
    }

    [[nodiscard]] bool is_child_of(Class* other)
    {
        if (other == this)
            return true;

        Class* next_parent = parent;
        while (next_parent)
        {
            if (next_parent == other)
                return true;
            next_parent = next_parent->parent;
        }
        return false;
    }

    template<typename Class_T = ObjectBase>
    [[nodiscard]] Class_T* instantiate() const
    {
        LOG_FATAL("NIY");
    }

  private:
    Class*              parent = nullptr;
    std::vector<Class*> children;

    const uint32_t type_size;
    const char*    class_name;
};

template <typename Class_T> [[nodiscard]] Class_T* cast(ObjectBase* other) requires std::is_base_of_v<ObjectBase, Class_T>
{
    Class* other_class = other->get_class();
    if (other_class->is_child_of(Class_T::static_class()) || Class_T::static_class()->is_child_of(other_class))
    {
        return static_cast<Class_T*>(other);
    }
    return nullptr;
}