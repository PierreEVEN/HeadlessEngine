

#include "rendering/shaders/shaders_builder.h"

#include "assets/asset_shader.h"
#include "rendering/shaders/shader_library.h"

#include <cpputils/logger.hpp>
#include <cpputils/stringutils.hpp>

static std::vector<std::string> split_code_lines(const std::string& code)
{
    std::vector<std::string> lines;
    std::string              current_line = "";
    for (const auto& chr : code)
    {
        if (chr == '\n')
        {
            lines.emplace_back(current_line);
            current_line.clear();
        }
        else
            current_line += chr;
    }
    if (!current_line.empty())
        lines.emplace_back(current_line);
    return lines;
}

ShaderPreprocessor::ShaderPreprocessor(const std::string& in_shader_code, const ShaderConfiguration& in_configuration) : shader_code(in_shader_code), configuration(in_configuration)
{
}

std::string ShaderPreprocessor::try_get_shader_code() const
{
    return generate_output_code(shader_code, configuration);
}

std::string ShaderPreprocessor::get_debug_code(uint32_t error_line, uint32_t error_column) const
{
    std::string debug_string;
    size_t      line_index = 0;
    for (const auto line : split_code_lines(try_get_shader_code()))
    {
        debug_string += stringutils::format("[ %d] : %s\n", ++line_index, line.c_str());
        if (error_line == line_index)
        {
            debug_string += "========";
            for (uint32_t i = 0; i < error_column; ++i)
            {
                debug_string += '=';
            }
            debug_string += "^================\n";
        }

    }
    return debug_string;
}

std::vector<std::string> ShaderPreprocessor::extract_includes(const std::string& shader_code)
{
    std::vector<std::string> includes   = {};
    const auto               code_lines = split_code_lines(shader_code);
    for (int i = 0; i < code_lines.size(); ++i)
    {
        if (const auto line = code_lines[i]; line.find("#include") != std::string::npos)
        {
            if (const auto split_include_line = stringutils::split(line, {'"'}); split_include_line.size() >= 2)
            {
                includes.emplace_back(split_include_line[1]);
            }
            else
            {
                LOG_ERROR("syntax error line %d : %s", i, line.c_str());
            }
        }
    }
    return includes;
}

std::string ShaderPreprocessor::replace_auto_locations_and_bindings(const std::string& shader_code, uint32_t& current_location, uint32_t& current_binding)
{
    std::string output_lines = "";
    const auto  code_lines   = split_code_lines(shader_code);

    for (const auto& line : code_lines)
    {
        if (const auto location = line.find("#AUTO_LOCATION"); location != std::string::npos)
        {
            output_lines += stringutils::format("layout (location = %d) %s\n", current_location++, line.substr(location + 15).c_str());
        }
        else if (const auto binding = line.find("#AUTO_BINDING"); binding != std::string::npos)
        {
            output_lines += stringutils::format("layout (binding = %d) %s\n", current_binding++, line.substr(binding + 14).c_str());
        }
        else
            output_lines += line + '\n';
    }

    return output_lines;
}

static std::string add_view_data_buffer(uint32_t& current_binding)
{
    return stringutils::format("// UNIFORM BUFFER\n"
                               "layout (binding = %d) uniform %s {\n"
                               "    mat4 worldProjection;\n"
                               "    mat4 viewMatrix;\n"
                               "    vec3 cameraLocation;\n"
                               "} SCENE_DATA_BUFFER_VAR;\n"
                               "\n"
                               "#define worldProjection SCENE_DATA_BUFFER_VAR.worldProjection\n"
                               "#define viewMatrix      SCENE_DATA_BUFFER_VAR.viewMatrix\n"
                               "#define cameraLocation  SCENE_DATA_BUFFER_VAR.cameraLocation\n",
                               current_binding++, G_SCENE_DATA_BUFFER_NAME);
}

static std::string add_scene_object_buffer(uint32_t& current_binding)
{
    return stringutils::format("struct INSTANCE_TRANSFORM_TYPE{\n"
                               "   mat4 model;\n"
                               "};\n"
                               "\n"
                               "layout (binding = %d) readonly buffer %s {\n"
                               "   INSTANCE_TRANSFORM_TYPE objects[];\n"
                               "} INSTANCE_TRANSFORM_DATA_VAR;\n"
                               "\n"
                               "#define instance_transform INSTANCE_TRANSFORM_DATA_VAR.objects \n",
                               current_binding++, G_MODEL_MATRIX_BUFFER_NAME);
}

std::string ShaderPreprocessor::generate_output_code(const std::string& shader_code, const ShaderConfiguration& configuration)
{
    uint32_t out_binding  = 0;
    uint32_t out_location = 0;

    // Global shader definition
    std::string generated_code = "// ########### GENERATED CODE ########### \n"
                                 "#version 460\n"
                                 "#extension GL_ARB_separate_shader_objects : enable \n\n";

    // Add input properties
    generated_code += "\n// input properties\n";
    if (configuration.input_stage)
    {
        for (const auto& input : configuration.input_stage->get_stage_outputs())
        {
            generated_code += stringutils::format("layout (location = %d) in %s %s;\n", input.location, input.get_property_glsl_typename().c_str(), input.property_name.c_str());
        }

        out_binding = configuration.input_stage->get_last_binding_index() + 1;
    }

    if (configuration.use_scene_object_buffer)
        generated_code += add_scene_object_buffer(out_binding);

    if (configuration.use_view_data_buffer)
        generated_code += add_view_data_buffer(out_binding);

    if (!configuration.properties.empty())
    {
        // Static data structure declaration
        generated_code += "\n// static data structure declaration\n";
        if (configuration.has_buffered_properties())
            generated_code += stringutils::format("layout(binding = %d) uniform %s\n", out_binding++, configuration.create_glsl_structure().c_str());
        for (const auto& property : configuration.properties)
        {
            if (property->should_keep_in_buffer_structure())
                generated_code += stringutils::format("#define %s %s.%s\n", property->get_property_name().c_str(), SHADER_STATIC_DATA_OBJECT_NAME, property->get_property_name().c_str());
            else
                generated_code += stringutils::format("layout (binding = %d) uniform %s %s;\n", out_binding++, property->get_glsl_type_name().c_str(), property->get_property_name().c_str());
        }
        generated_code += "\n";
    }

    // Handle includes
    generated_code += "\n// Include libraries\n";
    for (const auto& include : extract_includes(shader_code))
    {
        if (const auto library = ShaderLibrary::get_shader_library(include); library)
        {
            generated_code += stringutils::format("#ifndef INCLUDE_MODULE_%s\n"
                                                  "#define INCLUDE_MODULE_%s\n"
                                                  "%s\n"
                                                  "#endif\n",
                                                  include.c_str(), include.c_str(), library.value().c_str());
        }
        else
            LOG_ERROR("failed to find shader library named %s", include.c_str());
    }

    generated_code += "\n// ########### GENERATED CODE ###########\n\n";

    // add user raw code
    return replace_auto_locations_and_bindings(generated_code + get_raw_code(shader_code), out_location, out_binding);
}

std::string ShaderPreprocessor::get_raw_code(const std::string& shader_code)
{
    std::string output_lines = "";
    const auto  code_lines   = split_code_lines(shader_code);

    // get _raw code
    for (const auto& line : code_lines)
        if (line.find("#include") == std::string::npos)
            output_lines += line + "\n";

    return output_lines;
}
