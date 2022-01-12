#pragma once

#include "input_codes.h"

#include <cpputils/logger.hpp>
#include <types/property_binding.h>
#include <unordered_map>
#include <types/magic_enum.h>

namespace application::inputs
{
DECLARE_DELEGATE_MULTICAST(OnValueChanged);

class InputManager final
{
    friend class Key;

  public:
    static InputManager& get();

  private:

      InputManager()
      {
          for (const auto& entry : magic_enum::enum_entries<EButtons>())
              buttons[entry.first] = {};
          for (const auto& entry : magic_enum::enum_entries<EMouseAxis>())
              mouse_axis[entry.first] = {};
      }

    std::unordered_map<EButtons, OnValueChanged>   buttons;
    std::unordered_map<EMouseAxis, OnValueChanged> mouse_axis;
};

class Key
{
  public:
    Key(EButtons button) : on_value_changed(&InputManager::get().buttons[button])
    {
    }
    Key(EMouseAxis axis) : on_value_changed(&InputManager::get().mouse_axis[axis])
    {
    }

    OnValueChanged* on_value_changed;
};
} // namespace application