#pragma once

#include "shader_structures.h"
#include "types/fast_mutex.h"

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class ShaderModule final
{
  public:
    struct ShaderCompilationError
    {
        std::string error_string;
        uint32_t    error_line;
        uint32_t    error_column;
    };

    void set_bytecode(const std::vector<uint32_t>& in_bytecode);
    void set_plain_text(const std::string& in_shader_text);
    void set_shader_stage(VkShaderStageFlagBits in_shader_stage);

    [[nodiscard]] const std::vector<uint32_t>&          get_bytecode();
    [[nodiscard]] const VkShaderModule&                 get_shader_module();
    [[nodiscard]] std::optional<ShaderCompilationError> get_error();
    [[nodiscard]] bool                                  is_valid() const;
    ShaderModule() = default;
    ~ShaderModule();

  private:
    void create_shader_module();

    void destroy();
    void mark_dirty();

    std::vector<uint32_t> bytecode;

    std::optional<ShaderCompilationError> last_compilation_error = {};
    VkShaderStageFlagBits                 shader_stage           = VK_SHADER_STAGE_VERTEX_BIT;
    VkShaderModule                        module                 = VK_NULL_HANDLE;
    bool                                  b_is_dirty             = true;
    FastMutex                             access_lock;
};
