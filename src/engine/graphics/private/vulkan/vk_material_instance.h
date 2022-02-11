#pragma once
#include "vk_descriptor_pool.h"
#include "vulkan/vk_master_material.h"
#include "gfx/material_instance.h"

#include "vulkan/vk_unit.h"
#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class MaterialInstance_VK : public MaterialInstance
{
  public:
    MaterialInstance_VK(const std::shared_ptr<MasterMaterial>& base);
    void bind_buffer(const std::string& binding_name, const std::shared_ptr<Buffer>& in_buffer) override;

    bool bind_material(CommandBuffer* command_buffer) override;
    void bind_texture(const std::string& binding_name, const std::shared_ptr<Texture>& in_texture) override;
    void bind_sampler(const std::string& binding_name, const std::shared_ptr<Sampler>& in_sampler) override;

  private:
    [[nodiscard]] const shader_builder::BindingDescriptor* find_binding(const std::string& binding_name, const RenderPassID& render_pass) const;

  private:
    RenderPassData<SwapchainImageResource<TGpuHandle<DescriptorSetResource_VK>>> descriptor_sets;
    std::unordered_map<std::string, std::shared_ptr<Buffer>>                     write_buffers;
    std::unordered_map<std::string, std::shared_ptr<Texture>>                    write_textures;
    std::unordered_map<std::string, std::shared_ptr<Sampler>>                    write_samplers;
};

} // namespace gfx::vulkan
