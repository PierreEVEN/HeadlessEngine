#pragma once
#include "input_manager.h"

#include <cpputils/logger.hpp>

namespace application::inputs
{

class BaseMapping
{
  public:
    BaseMapping(Key in_key, std::vector<Key> in_modifiers) : key(in_key), modifiers(std::move(in_modifiers))
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

    Key              key;
    std::vector<Key> modifiers;

  private:
    void value_changed_internal()
    {
        value_changed();
    }
};

class ActionMapping final : public BaseMapping
{
  public:
    ActionMapping(Key in_key, const std::vector<Key>& in_modifiers = {}) : BaseMapping(in_key, in_modifiers), value(false)
    {
    }
    void value_changed() override
    {
        bool final_value = key.get_bool_value();
        for (const auto& modifier : modifiers)
            if (!modifier.get_bool_value())
            {
                final_value = false;
                break;
            }
        if (value != final_value)
            value = final_value;
    }

    PropertyContainer<bool> value;
};

class AxisMapping final : public BaseMapping
{
  public:
    AxisMapping(Key in_key, const std::vector<Key>& in_modifiers = {}) : BaseMapping(in_key, in_modifiers), value(0)
    {
    }
    void value_changed() override
    {
        for (const auto& modifier : modifiers)
            if (!modifier.get_bool_value())
            {
                return;
            }

        const float final_value = key.get_float_value();
        if (value != final_value)
            value = final_value;
    }

    PropertyContainer<float> value;
};
} // namespace application::inputs