

#include "vulkan/vk_descriptor_pool.h"

#include "vk_helper.h"
#include "vk_master_material.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"

#include <array>
#include <cpputils/logger.hpp>

namespace gfx::vulkan
{

DescriptorSetLayoutResource_VK::DescriptorSetLayoutResource_VK(const std::string& name, const CI_DescriptorSetLayout& create_infos)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& binding : create_infos.vertex_reflection_data.bindings)
    {
        bindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding            = binding.binding,
            .descriptorType     = MasterMaterial_VK::vk_descriptor_type(binding.descriptor_type),
            .descriptorCount    = 1,
            .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        });
    }

    for (const auto& binding : create_infos.fragment_reflection_data.bindings)
    {
        bindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding            = binding.binding,
            .descriptorType     = MasterMaterial_VK::vk_descriptor_type(binding.descriptor_type),
            .descriptorCount    = 1,
            .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        });
    }

    VkDescriptorSetLayoutCreateInfo layout_infos{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings    = bindings.data(),
    };
    VK_CHECK(vkCreateDescriptorSetLayout(get_device(), &layout_infos, get_allocator(), &descriptor_set_layout), "Failed to create descriptor set layout");
    debug_set_object_name(name, descriptor_set_layout);
}

DescriptorSetLayoutResource_VK::~DescriptorSetLayoutResource_VK()
{
    vkDestroyDescriptorSetLayout(get_device(), descriptor_set_layout, get_allocator());
}

DescriptorSetResource_VK::DescriptorSetResource_VK(const std::string&, const CI_DescriptorSet& create_infos) : is_dirty(true), parameters(create_infos)
{
    descriptor_set = parameters.descriptor_pool->allocate(create_infos.desc_set_layout);
}

DescriptorSetResource_VK::~DescriptorSetResource_VK()
{
    parameters.descriptor_pool->free(descriptor_set);
}

TGpuHandle<DescriptorSetResource_VK> DescriptorPool_VK::create_descriptor_set(TGpuHandle<DescriptorSetLayoutResource_VK> desc_set_layout)
{
    TGpuHandle<DescriptorPoolResource_VK> selected_pool;
    for (const auto& pool : context_pools)
    {
        if (pool)
        {
            selected_pool = pool;
            break;
        }
    }
    if (!selected_pool)
        selected_pool = TGpuHandle<DescriptorPoolResource_VK>("descriptor pool", DescriptorPoolResource_VK::CI_DescriptorPool{});

    return TGpuHandle<DescriptorSetResource_VK>("descriptor set", DescriptorSetResource_VK::CI_DescriptorSet{
                                                                      .desc_set_layout = desc_set_layout,
                                                                      .descriptor_pool = selected_pool,
                                                                  });
}

DescriptorPoolResource_VK::DescriptorPoolResource_VK(const std::string& name, const CI_DescriptorPool& create_infos) : space_left(VK_MAX_DESCRIPTOR_PER_POOL), pool_thread_id(std::this_thread::get_id())
{
    const std::initializer_list<VkDescriptorPoolSize> poolSizes{
        {VK_DESCRIPTOR_TYPE_SAMPLER, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_MAX_DESCRIPTOR_PER_TYPE},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_MAX_DESCRIPTOR_PER_TYPE},
    };

    const VkDescriptorPoolCreateInfo poolInfo{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = static_cast<VkDescriptorPoolCreateFlags>(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) | (create_infos.bindless_pool ? VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT : NULL),
        .maxSets       = VK_MAX_DESCRIPTOR_PER_POOL,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes    = poolSizes.begin(),
    };

    VK_CHECK(vkCreateDescriptorPool(get_device(), &poolInfo, get_allocator(), &pool), "Failed to create descriptor pool");
}

DescriptorPoolResource_VK::~DescriptorPoolResource_VK()
{
    vkDestroyDescriptorPool(get_device(), pool, get_allocator());
}

DescriptorPoolResource_VK::operator bool() const
{
    return (pool_thread_id == std::this_thread::get_id() && space_left > 0);
}

VkDescriptorSet DescriptorPoolResource_VK::allocate(const TGpuHandle<DescriptorSetLayoutResource_VK>& layout)
{
    space_left--;
    const VkDescriptorSetAllocateInfo descriptor_info{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &layout->descriptor_set_layout,
    };
    VkDescriptorSet descriptor_set;
    VK_CHECK(vkAllocateDescriptorSets(get_device(), &descriptor_info, &descriptor_set), "failed to allocate descriptor sets");
    return descriptor_set;
}

void DescriptorPoolResource_VK::free(VkDescriptorSet descriptor)
{
    vkFreeDescriptorSets(get_device(), pool, 1, &descriptor);
    space_left++;
}
} // namespace gfx::vulkan