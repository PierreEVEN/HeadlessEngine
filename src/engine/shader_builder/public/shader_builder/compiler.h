#pragma once
#include <string>

#include "operation.h"

namespace shader_builder
{
struct CompilationResult
{
    OperationStatus status;

};

CompilationResult compile_shader(const std::string& file_path);

}
