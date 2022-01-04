#pragma once

namespace shader_builder
{
class CustomIncluder;

void init();
void destroy();

class ShaderBuilder final
{
    friend ShaderBuilder* get();

  public:
    ~ShaderBuilder();

    void compile_shader();

    [[nodiscard]] CustomIncluder* get_includer() const
    {
        return includer;
    }

  private:
    CustomIncluder*           includer = nullptr;
    ShaderBuilder();
};

ShaderBuilder* get();

} // namespace shader_builder
