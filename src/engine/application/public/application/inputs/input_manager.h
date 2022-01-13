#pragma once

#define MAGIC_ENUM_RANGE_MAX 1024
#include <types/magic_enum.h>

#include "input_codes.h"

#include <cpputils/logger.hpp>
#include <types/property_binding.h>
#include <unordered_map>

namespace application::inputs
{
DECLARE_DELEGATE_MULTICAST(OnValueChanged);

class InputManager final
{
    friend class Key;
  public:
    struct ButtonValue
    {
        bool           pressed;
        bool           first_pressed;
        OnValueChanged value_changed;
    };
    struct AxisValue
    {
        float          value;
        OnValueChanged value_changed;
    };

    static InputManager& get();

    void press_button(EButtons key);
    void release_button(EButtons key);
    void move_axis(EAxis key, float value);

  private:
    InputManager();

    std::unordered_map<EButtons, ButtonValue> buttons;
    std::unordered_map<EAxis, AxisValue>      axis;
};

class Key final
{
  public:
    Key(EButtons in_button) : on_value_changed(&InputManager::get().buttons[button].value_changed), is_button(true), button(in_button)
    {
    }
    Key(EAxis in_axis) : on_value_changed(&InputManager::get().axis[axis].value_changed), is_button(false), axis(in_axis)
    {
    }

    [[nodiscard]] bool get_bool_value() const
    {
        if (is_button)
            return InputManager::get().buttons[button].pressed;
        else
            return abs(InputManager::get().axis[axis].value) > 0.5f;
    }

    [[nodiscard]] float get_float_value() const
    {
        if (is_button)
            return InputManager::get().buttons[button].pressed ? 1.f : 0.f;
        else
            return InputManager::get().axis[axis].value;
    }

    bool is_button;
    EButtons button;
    EAxis axis;

    OnValueChanged* on_value_changed;
};
} // namespace application::inputs