#pragma once
#include "input_manager.h"

#include <cpputils/logger.hpp>

namespace application::inputs
{

class BaseMapping
{
  public:
    BaseMapping(Key in_key, const std::vector<Key>& in_modifiers) : key(in_key), modifiers(in_modifiers)
    {
        key.on_value_changed->add_object(this, &BaseMapping::value_changed_internal);
        for (const auto& modifier : modifiers)
            modifier.on_value_changed->add_object(this, &BaseMapping::value_changed_internal);
    }
    virtual ~BaseMapping()
    {
        key.on_value_changed->clear_object(this);
        for (const auto& modifier : modifiers)
            modifier.on_value_changed->clear_object(this);
    }

  protected:
    virtual void value_changed() = 0;

  private:
    void value_changed_internal()
    {
        value_changed();
    }

    Key              key;
    std::vector<Key> modifiers;
};

class ActionMapping final : public BaseMapping
{
  public:
    ActionMapping(Key key, const std::vector<Key>& modifiers = {}) : BaseMapping(key, modifiers)
    {
    }
    void value_changed() override
    {
    }

    PropertyContainer<bool> value;
};

class AxisMapping final : public BaseMapping
{
  public:
    AxisMapping(Key key, const std::vector<Key>& modifiers = {}) : BaseMapping(key, modifiers)
    {
    }
    void value_changed() override
    {
    }

    PropertyContainer<float> value;
};
} // namespace application::inputs