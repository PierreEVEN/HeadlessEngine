#pragma once
#include "asset_base.h"
#include "rendering/vulkan/shader_module.h"

#include <optional>
#include <spirv_cross.hpp>
#include <vulkan/vulkan_core.h>

constexpr const char* G_SCENE_DATA_BUFFER_NAME   = "GlobalCameraUniformBuffer";
constexpr const char* G_MODEL_MATRIX_BUFFER_NAME = "ObjectBuffer";

struct ShaderProperty
{
    std::string                     property_name;
    spirv_cross::SPIRType::BaseType property_type;
    size_t                          structure_size;
    uint32_t                        location;
    uint32_t                        vec_size;
    EShaderStage                    shader_stage;

    std::vector<ShaderProperty> structure_properties;
};

class AShader : public AssetBase
{
  public:
    AShader(const std::filesystem::path& source_mesh_path, EShaderStage in_shader_kind);
    virtual ~AShader() override = default;

    [[nodiscard]] VkShaderModule          get_shader_module() const;
    [[nodiscard]] ShaderModule* get_shader_module_ptr() const;

    [[nodiscard]] const std::optional<ShaderProperty>& get_push_constants() const;
    [[nodiscard]] const std::vector<ShaderProperty>&   get_uniform_buffers() const;
    [[nodiscard]] const std::vector<ShaderProperty>&   get_image_samplers() const;
    [[nodiscard]] const std::vector<ShaderProperty>&   get_storage_buffers() const;

    [[nodiscard]] const ShaderProperty* get_model_matrix_buffer() const;
    [[nodiscard]] const ShaderProperty* get_scene_data_buffer() const;

  private:
    void                       build_reflection_data(const std::vector<uint32_t>& bytecode);
    std::optional<std::string> read_shader_file(const std::filesystem::path& source_path);

    const EShaderStage shader_stage;

    std::unique_ptr<ShaderModule> shader_module;

    // Reflection data
    std::string                   entry_point;
    std::optional<ShaderProperty> push_constants = std::optional<ShaderProperty>();
    std::vector<ShaderProperty>   uniform_buffer = {};
    std::vector<ShaderProperty>   storage_buffer = {};
    std::vector<ShaderProperty>   image_samplers = {};
};