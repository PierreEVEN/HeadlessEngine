#pragma once

#include <cpputils/eventmanager.hpp>

template <typename Property_T> class PropertyContainer final
{
  public:
    PropertyContainer()  = default;
    ~PropertyContainer()                              = default;
    PropertyContainer(const PropertyContainer& other) = delete;
    PropertyContainer(PropertyContainer&& other)      = delete;
    PropertyContainer& operator=(PropertyContainer&& other) = delete;
    PropertyContainer& operator=(const PropertyContainer& other) = delete;
    
    PropertyContainer(const Property_T& value) : Property_T(value)
    {
    }

    PropertyContainer& operator=(const Property_T& other)
    {
        value = other;
        value_changed.execute(value);
        return *this;
    }
    PropertyContainer& operator=(const Property_T&& other)
    {
        value = std::move(other);
        value_changed.execute(value);
        return *this;
    }
    
    bool operator==(const PropertyContainer<Property_T>& other) const
    {
        return value == other.value;
    }

    const Property_T* operator->() const
    {
        return &value;
    }

    Property_T* operator->()
    {
        return &value;
    }

    const Property_T& operator*() const
    {
        return value;
    }

    Property_T& operator*()
    {
        return value;
    }

    DelegateMultiCast<Property_T> value_changed;
    
  private:
    Property_T value;
};