#pragma once
#include <filesystem>
#include <optional>
#include <string>

class ShaderLibrary
{
  public:
    ShaderLibrary() = delete;
    static void                                     register_from_file(const std::string& name, const std::filesystem::path& shl_file);
    static void                                     register_from_code(const std::string& name, const std::string& glsl_code);
    [[nodiscard]] static std::optional<std::string> get_shader_library(const std::string& name);
};
