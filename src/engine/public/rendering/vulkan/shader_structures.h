#pragma once

#include "assets/asset_ptr.h"

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

class ShaderParameter
{
  public:
    template <typename Parameter_T, typename... Args_T> static std::shared_ptr<ShaderParameter> create(const std::string& name, VkShaderStageFlagBits stage, Args_T&&... arguments)
    {
        std::shared_ptr<ShaderParameter> parameter = std::make_shared<Parameter_T>(std::forward<Args_T>(arguments)...);
        parameter->parameter_name                  = name;
        parameter->shader_stage                    = stage;
        parameter->mark_dirty();
        return parameter;
    }

    void mark_dirty()
    {
        for (int i = 0; i < dirty_buffers.size(); ++i)
        {
            dirty_buffers[i] = true;
        }
    }

    virtual VkWriteDescriptorSet generate_descriptor_sets() = 0;

  protected:
    ShaderParameter() = default;

  private:
    std::string       parameter_name;
    VkShaderStageFlagBits shader_stage;
    std::vector<bool> dirty_buffers;
};

struct ShaderStageConfiguration
{
    TAssetPtr<class AShader>                                        shader            = {};
    std::unordered_map<std::string, TAssetPtr<class AShaderBuffer>> uniform_buffers   = {};
    std::unordered_map<std::string, TAssetPtr<class AShaderBuffer>> storage_buffers   = {};
    std::unordered_map<std::string, TAssetPtr<class ATexture>>      textures          = {};
};

