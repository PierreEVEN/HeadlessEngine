#include "shader_builder/shader_builder.h"

#include <cpputils/logger.hpp>
#include <glslang/Public/ShaderLang.h>
#include <internal.h>
#include <string>

#include <glslang/SPIRV/GlslangToSpv.h>
#include "custom_includer.h"

namespace shader_builder
{

std::unique_ptr<ShaderBuilder> singleton;

ShaderBuilder* get()
{
    if (!singleton)
        singleton = std::unique_ptr<ShaderBuilder>(new ShaderBuilder());
    return singleton.get();
}
void destroy()
{
    singleton = nullptr;
}

ShaderBuilder::~ShaderBuilder()
{
    delete includer;
    glslang::FinalizeProcess();
}

ShaderBuilder::ShaderBuilder()
{
    if (!glslang::InitializeProcess())
        LOG_FATAL("failed to initialize glslang");
    init_resources();
    includer = new CustomIncluder(this);
}

} // namespace shader_builder
