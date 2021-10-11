#pragma once
#include "asset_base.h"
#include "rendering/mesh/vertex.h"
#include "rendering/shaders/shader_property.h"
#include "rendering/vulkan/shader_module.h"

#include <optional>
#include <spirv_cross.hpp>
#include <vulkan/vulkan_core.h>

constexpr const char* G_SCENE_DATA_BUFFER_NAME   = "SCENE_DATA_BUFFER";
constexpr const char* G_MODEL_MATRIX_BUFFER_NAME = "INSTANCE_TRANSFORM_DATA";

struct ShaderReflectProperty
{
    std::string                     property_name;
    spirv_cross::SPIRType::BaseType property_type;
    size_t                          structure_size;
    uint32_t                        location;
    uint32_t                        vec_size;
    VkShaderStageFlagBits           shader_stage;

    std::vector<ShaderReflectProperty> structure_properties;

    [[nodiscard]] std::string get_property_glsl_typename() const;
};

class AShader : public AssetBase
{
  public:
    AShader(const std::filesystem::path& source_mesh_path, const ShaderInfos& in_shader_configuration, const TAssetPtr<AShader>& input_stage = {});
    AShader(const std::vector<uint32_t>& shader_bytecode, const ShaderInfos& in_shader_configuration);
    virtual ~AShader() override = default;

    [[nodiscard]] VkShaderModule get_shader_module() const;
    [[nodiscard]] ShaderModule*  get_shader_module_ptr() const;

    [[nodiscard]] const std::optional<ShaderReflectProperty>& get_push_constants() const;
    [[nodiscard]] const std::vector<ShaderReflectProperty>&   get_uniform_buffers() const;
    [[nodiscard]] const std::vector<ShaderReflectProperty>&   get_image_samplers() const;
    [[nodiscard]] const std::vector<ShaderReflectProperty>&   get_storage_buffers() const;
    [[nodiscard]] const std::vector<ShaderReflectProperty>&   get_stage_inputs() const;
    [[nodiscard]] const std::vector<ShaderReflectProperty>&   get_stage_outputs() const;
    [[nodiscard]] std::vector<ShaderReflectProperty>          get_all_properties() const;
    [[nodiscard]] std::optional<ShaderReflectProperty>        find_property_by_name(const std::string& property_name) const;

    [[nodiscard]] const ShaderReflectProperty* get_model_matrix_buffer() const;
    [[nodiscard]] const ShaderReflectProperty* get_scene_data_buffer() const;

    [[nodiscard]] const VkShaderStageFlagBits& get_shader_stage() const;
    [[nodiscard]] const ShaderInfos&   get_shader_config() const;
    [[nodiscard]] uint32_t                     get_last_binding_index() const;

  private:
    void                       build_reflection_data(const std::vector<uint32_t>& bytecode);
    std::optional<std::string> read_shader_file(const std::filesystem::path& source_path);

    const ShaderInfos     shader_configuration;
    std::unique_ptr<ShaderModule> shader_module;

    // Reflection data
    uint32_t                             last_binding_index = 0;
    std::string                          entry_point        = "main";
    std::optional<ShaderReflectProperty> push_constants     = std::optional<ShaderReflectProperty>();
    std::vector<ShaderReflectProperty>   uniform_buffer     = {};
    std::vector<ShaderReflectProperty>   storage_buffer     = {};
    std::vector<ShaderReflectProperty>   image_samplers     = {};
    std::vector<ShaderReflectProperty>   shader_inputs      = {};
    std::vector<ShaderReflectProperty>   shader_outputs     = {};
};