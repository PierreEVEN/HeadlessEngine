#pragma once
#include <filesystem>
#include <set>

namespace shader_builder
{
class CustomIncluder;

void init();
void destroy();

enum class ShaderStage
{
    
};

struct SHaderCompileResult
{
    
};

class ShaderBuilder final
{
    friend ShaderBuilder* get();

  public:
    void add_include_dir(const std::filesystem::path& dir);
    ~ShaderBuilder();

    void compile_shader();

    [[nodiscard]] CustomIncluder* get_includer() const
    {
        return includer;
    }

  private:
    std::set<std::filesystem::path> include_paths;
    CustomIncluder*           includer = nullptr;
    ShaderBuilder();
};

ShaderBuilder* get();

} // namespace shader_builder
