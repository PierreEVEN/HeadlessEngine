#pragma once

#include "gfx/resource/gpu_resource.h"
#include "shader_builder/shader_types.h"

#include <thread>
#include <vulkan/vulkan.h>

namespace gfx::vulkan
{
class DescriptorSetLayoutResource_VK final
{
  public:
    struct CreateInfos
    {
        const std::vector<shader_builder::BindingDescriptor>& vertex_bindings;
        const std::vector<shader_builder::BindingDescriptor>& fragment_bindings;
        bool                                                  bindless_descriptor_set = false;
    };

    DescriptorSetLayoutResource_VK(const std::string& name, const CreateInfos& create_infos);
    ~DescriptorSetLayoutResource_VK();

    VkDescriptorSetLayout descriptor_set_layout;
};

class DescriptorPoolResource_VK final
{
  public:
    struct CreateInfos
    {
        uint32_t max_descriptor_per_pool = 64;
        uint32_t max_descriptor_per_type = 256;
        bool     bindless_pool           = false;
    };

    DescriptorPoolResource_VK(const std::string& name, const CreateInfos& create_infos);
    ~DescriptorPoolResource_VK();

    // Return true if a descriptor can be currently allocated using this pool
    explicit operator bool() const;

    // Allocate a new descriptor set in this pool
    VkDescriptorSet allocate(const TGpuHandle<DescriptorSetLayoutResource_VK>& layout);
    // Free a descriptor allocated in this pool
    void free(VkDescriptorSet descriptor);

  private:
    VkDescriptorPool pool;
    uint32_t         space_left;
    std::thread::id  pool_thread_id;
};

class DescriptorSetResource_VK final
{
  public:
    struct CreateInfos
    {
        TGpuHandle<DescriptorSetLayoutResource_VK> desc_set_layout;
        TGpuHandle<DescriptorPoolResource_VK>      descriptor_pool;
    };
    DescriptorSetResource_VK(const std::string&, const CreateInfos& create_infos);
    ~DescriptorSetResource_VK();

    // Was resource updated
    bool                              is_dirty;
    std::vector<VkWriteDescriptorSet> write_descriptor_sets;
    VkDescriptorSet                   descriptor_set;

  private:
    const TGpuHandle<DescriptorSetLayoutResource_VK> descriptor_set_layout;
    TGpuHandle<DescriptorPoolResource_VK>            descriptor_pool;
};
} // namespace gfx::vulkan