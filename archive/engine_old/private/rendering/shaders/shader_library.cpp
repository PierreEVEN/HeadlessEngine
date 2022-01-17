

#include "rendering/shaders/shader_library.h"

#include <cpputils/logger.hpp>
#include <unordered_map>

std::unordered_map<std::string, std::string> registered_libraries;

static std::optional<std::string> read_shader_file(const std::filesystem::path& source_path)
{
    std::optional<std::string> code;

    if (exists(source_path))
    {
        std::ifstream shader_file(source_path);

        std::string code_string;
        std::string line;
        while (std::getline(shader_file, line))
        {
            code_string += line + "\n";
        }

        shader_file.close();
        code = code_string;
    }

    return code;
}

void ShaderLibrary::register_from_file(const std::string& name, const std::filesystem::path& shl_file)
{
    if (const auto code = read_shader_file(shl_file); code)
    {
        register_from_code(name, code.value());
    }
    else
        LOG_ERROR("cannot register shader library %s : file %s does not exists", name.c_str(), shl_file.string().c_str());


}

void ShaderLibrary::register_from_code(const std::string& name, const std::string& glsl_code)
{
    if (get_shader_library(name))
        LOG_FATAL("There already is a shader library named %s", name.c_str());
    registered_libraries[name] = glsl_code;
}

std::optional<std::string> ShaderLibrary::get_shader_library(const std::string& name)
{
    if (const auto& found_library = registered_libraries.find(name); found_library != registered_libraries.end())
        return found_library->second;

    return std::optional<std::string>();
}
