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
    static InputManager& get();

    void press_button(EButtons key);
    void release_button(EButtons key);
    void move_axis(EAxis key, int value);

  private:
    InputManager()
    {
        for (const auto& entry : magic_enum::enum_entries<EButtons>())
            buttons[entry.first] = {};
        for (const auto& entry : magic_enum::enum_entries<EAxis>())
            mouse_axis[entry.first] = {};
    }

    std::unordered_map<EButtons, OnValueChanged> buttons;
    std::unordered_map<EAxis, OnValueChanged>    mouse_axis;
};

class Key
{
  public:
    Key(EButtons button) : on_value_changed(&InputManager::get().buttons[button])
    {
    }
    Key(EAxis axis) : on_value_changed(&InputManager::get().mouse_axis[axis])
    {
    }

    OnValueChanged* on_value_changed;
};
} // namespace application::inputs