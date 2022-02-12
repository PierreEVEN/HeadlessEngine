

#include "vk_resource_descriptors.h"

#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_master_material.h"

namespace gfx::vulkan
{
DescriptorSetLayoutResource_VK::DescriptorSetLayoutResource_VK(const std::string& name, const CreateInfos& create_infos)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& binding : create_infos.vertex_bindings)
    {
        bindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding            = binding.binding,
            .descriptorType     = MasterMaterial_VK::vk_descriptor_type(binding.descriptor_type),
            .descriptorCount    = 1,
            .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        });
    }

    for (const auto& binding : create_infos.fragment_bindings)
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

DescriptorSetResource_VK::DescriptorSetResource_VK(const std::string&, const CreateInfos& create_infos) : is_dirty(true), descriptor_set_layout(create_infos.desc_set_layout), descriptor_pool(create_infos.descriptor_pool)
{
    descriptor_set = descriptor_pool->allocate(create_infos.desc_set_layout);
}

DescriptorSetResource_VK::~DescriptorSetResource_VK()
{
    descriptor_pool->free(descriptor_set);
}

DescriptorPoolResource_VK::DescriptorPoolResource_VK(const std::string& name, const CreateInfos& create_infos) : space_left(create_infos.max_descriptor_per_pool), pool_thread_id(std::this_thread::get_id())
{
    const std::initializer_list<VkDescriptorPoolSize> poolSizes{
        {VK_DESCRIPTOR_TYPE_SAMPLER, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, create_infos.max_descriptor_per_type},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, create_infos.max_descriptor_per_type},
    };

    const VkDescriptorPoolCreateInfo poolInfo{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = static_cast<VkDescriptorPoolCreateFlags>(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) | (create_infos.bindless_pool ? VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT : NULL),
        .maxSets       = create_infos.max_descriptor_per_pool,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes    = poolSizes.begin(),
    };

    VK_CHECK(vkCreateDescriptorPool(get_device(), &poolInfo, get_allocator(), &pool), "Failed to create descriptor pool");
    debug_set_object_name(name, pool);
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