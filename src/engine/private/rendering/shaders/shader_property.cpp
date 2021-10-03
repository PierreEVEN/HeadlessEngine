

#include "rendering/shaders/shader_property.h"

#include <cpputils/stringutils.hpp>

bool ShaderConfiguration::has_buffered_properties() const
{
    for (const auto& property : properties)
        if (property.should_keep_in_buffer_structure())
            return true;
    return false;
}

std::string ShaderConfiguration::create_glsl_structure() const
{
    std::string output_string = stringutils::format("%s_TYPE {\n", SHADER_STATIC_DATA_OBJECT_NAME);

    for (const auto& property : properties)
    {
        if (property.should_keep_in_buffer_structure())
            output_string += stringutils::format("\t%s %s;\n", property.get_glsl_type_name().c_str(), property.get_property_name().c_str());
    }
    return output_string + stringutils::format("} %s;\n", SHADER_STATIC_DATA_OBJECT_NAME);
}
