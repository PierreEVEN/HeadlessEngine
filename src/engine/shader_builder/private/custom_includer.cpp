

#include "custom_includer.h"

#include <cpputils/logger.hpp>

namespace shader_builder
{
CustomIncluder::CustomIncluder(ShaderBuilder* in_owner) : owner(in_owner)
{
}

void CustomIncluder::add_include_path(const std::filesystem::path& include_path)
{
    LOG_WARNING("add include path");
    include_paths.emplace_back(include_path);
}

glslang::TShader::Includer::IncludeResult* CustomIncluder::includeSystem([[maybe_unused]] const char* headerName, [[maybe_unused]] const char* includerName, [[maybe_unused]] size_t inclusionDepth)
{
    //LOG_WARNING("try include system %s : %s : %d", headerName, includerName, inclusionDepth);
    // System include is not handled
    return nullptr;
}

glslang::TShader::Includer::IncludeResult* CustomIncluder::includeLocal(const char* headerName, const char* includerName, [[maybe_unused]] size_t inclusionDepth)
{
    const auto base_directory = std::filesystem::path(includerName).parent_path();

    if (is_regular_file(base_directory / std::filesystem::path(headerName)))
        return make_include_result(base_directory / std::filesystem::path(headerName));

    for (const auto& path : include_paths)
        if (is_regular_file(base_directory / path))
            return make_include_result(base_directory / path);

    return nullptr;
}

void CustomIncluder::releaseInclude(IncludeResult* include)
{
    if (include)
    {
        delete[] include->headerData;
        delete include;
    }
}

glslang::TShader::Includer::IncludeResult* CustomIncluder::make_include_result(const std::filesystem::path& path)
{
    std::string shader_code;
    std::string line;

    std::ifstream str(path);

    if (!str)
        return nullptr;

    while (std::getline(str, line))
        shader_code += line + "\n";

    const auto header_data = new char[shader_code.size() + 1];
    memcpy(header_data, shader_code.c_str(), shader_code.size());
    header_data[shader_code.size()] = '\0';

    return new IncludeResult(path.string(), header_data, shader_code.size(), nullptr);
}
} // namespace shader_builder
