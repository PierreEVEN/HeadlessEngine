#pragma once
#pragma once

#include "gfx/resource/gpu_resource.h"
#include "shader_builder/shader_types.h"

#include <mutex>
#include <thread>
#include <vector>
#include <vulkan/vulkan.h>

#ifndef VK_MAX_DESCRIPTOR_PER_POOL
#define VK_MAX_DESCRIPTOR_PER_POOL 64
#endif
#ifndef VK_MAX_DESCRIPTOR_PER_TYPE
#define VK_MAX_DESCRIPTOR_PER_TYPE 256
#endif

namespace gfx::vulkan
{
class DescriptorSetLayoutResource_VK final
{
  public:
    struct CI_DescriptorSetLayout
    {
        const shader_builder::ReflectionResult& vertex_reflection_data;
        const shader_builder::ReflectionResult& fragment_reflection_data;
    };

    DescriptorSetLayoutResource_VK(const std::string& name, const CI_DescriptorSetLayout& create_infos);
    ~DescriptorSetLayoutResource_VK();

    VkDescriptorSetLayout descriptor_set_layout;
};

class DescriptorPoolResource_VK final
{
  public:
    struct CI_DescriptorPool
    {
        bool bindless_pool = false;
    };

    DescriptorPoolResource_VK(const std::string& name, const CI_DescriptorPool& create_infos);
    ~DescriptorPoolResource_VK();

    explicit operator bool() const;

    VkDescriptorSet allocate(const TGpuHandle<DescriptorSetLayoutResource_VK>& layout);
    void            free(VkDescriptorSet descriptor);

  private:
    VkDescriptorPool pool;
    uint32_t         space_left;
    std::thread::id  pool_thread_id;
};

class DescriptorSetResource_VK final
{
  public:
    struct CI_DescriptorSet
    {
        TGpuHandle<DescriptorSetLayoutResource_VK> desc_set_layout;
        TGpuHandle<DescriptorPoolResource_VK>      descriptor_pool;
    };
    DescriptorSetResource_VK(const std::string&, const CI_DescriptorSet& create_infos);
    ~DescriptorSetResource_VK();
    bool                              is_dirty;
    std::vector<VkWriteDescriptorSet> write_descriptor_sets;
    VkDescriptorSet                   descriptor_set;

  private:
    CI_DescriptorSet parameters;
};

class DescriptorPool_VK final
{
  public:
    TGpuHandle<DescriptorSetResource_VK> create_descriptor_set(TGpuHandle<DescriptorSetLayoutResource_VK> desc_set_layout);

  private:
    std::vector<TGpuHandle<DescriptorPoolResource_VK>> context_pools;
    std::mutex                                         find_pool_lock;
};
} // namespace gfx::vulkan