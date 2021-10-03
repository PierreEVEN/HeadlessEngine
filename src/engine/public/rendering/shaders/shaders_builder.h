#pragma once

#include "shader_property.h"

#include <string>
#include <vector>

class ShaderPreprocessor
{
  public:
    ShaderPreprocessor(const std::string& in_shader_code, const ShaderConfiguration& in_configuration);

    [[nodiscard]] std::string try_get_shader_code() const;
    [[nodiscard]] std::string get_debug_code(uint32_t error_line, uint32_t error_column) const;

  private:
    [[nodiscard]] static std::vector<std::string> extract_includes(const std::string& shader_code);

    [[nodiscard]] static std::string replace_auto_locations_and_bindings(const std::string& shader_code, uint32_t& current_location, uint32_t& current_binding);
    [[nodiscard]] static std::string generate_output_code(const std::string& shader_code, const ShaderConfiguration& configuration);
    [[nodiscard]] static std::string get_raw_code(const std::string& shader_code);

    const std::string         shader_code;
    const ShaderConfiguration configuration;
};
