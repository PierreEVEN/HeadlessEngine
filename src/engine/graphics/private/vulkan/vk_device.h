#pragma once

#include "gfx/resource/device.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#define get_device() ((Device_VK*)&Device::get())->get_device_temp_internal_afac()

#define get_vma_allocator() ((Device_VK*)&Device::get())->get_vma_allocator_temp_internal_afac()

namespace gfx::vulkan
{
class Device_VK final : public Device
{
  public:
    void init() override;
    Device_VK(uint8_t image_count) : Device(image_count)
    {
    }
    ~Device_VK() override;

    BufferHandle        create_buffer(const std::string& name, const CI_Buffer& create_infos) override;
    CommandBufferHandle create_command_buffer(const std::string& name, const CI_CommandBuffer& create_infos) override;
    BufferViewHandle    create_buffer_view(const std::string& name, const CI_BufferView& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    QueueHandle create_queue(const std::string& name, const CI_Queue& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    SemaphoreHandle create_semaphore(const std::string& name, const CI_Semaphore& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    FenceHandle create_fence(const std::string& name, const CI_Fence& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    TextureHandle     create_texture(const std::string& name, const CI_Texture& create_infos) override;
    TextureViewHandle create_texture_view(const std::string& name, const CI_TextureView& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    ShaderHandle create_shader(const std::string& name, const CI_Shader& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    SamplerHandle create_sampler(const std::string& name, const CI_Sampler& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    PipelineLayoutHandle create_pipeline_layout(const std::string& name, const CI_PipelineLayout& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    RenderPassHandle create_render_pass(const std::string& name, const CI_RenderPass& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    PipelineHandle create_pipeline(const std::string& name, const CI_Pipeline& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    DescriptorSetLayoutHandle create_descriptor_set_layout(const std::string& name, const CI_DescriptorSetLayout& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    FramebufferHandle create_framebuffer(const std::string& name, const CI_Framebuffer& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    DescriptorPoolHandle create_descriptor_pool(const std::string& name, const CI_DescriptorPool& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    CommandPoolHandle create_command_pool(const std::string& name, const CI_CommandPool& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    SurfaceHandle create_surface(const std::string& name, const CI_Surface& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
    SwapchainHandle create_swapchain(const std::string& name, const CI_Swapchain& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }

    [[nodiscard]] VkDevice get_device_temp_internal_afac() const
    {
        return device;
    }

    [[nodiscard]] const VmaAllocator& get_vma_allocator_temp_internal_afac() const
    {
        return vulkan_memory_allocator;
    }

    void wait_device() override;

  private:
    VkDevice     device;
    VmaAllocator vulkan_memory_allocator;
};

class BufferResource_VK final
{
  public:
    BufferResource_VK(const std::string& name, const CI_Buffer& create_infos);
    ~BufferResource_VK();

    [[nodiscard]] VkBuffer& get_buffer()
    {
        return buffer;
    }

    [[nodiscard]] VmaAllocation& get_allocation()
    {
        return memory;
    }

    void* map_buffer();
    void  unmap_buffer();

  private:
    VkBuffer      buffer = VK_NULL_HANDLE;
    VmaAllocation memory = VK_NULL_HANDLE;
};

class ImageResource_VK final
{
  public:
    ImageResource_VK(const std::string& name, const CI_Texture& create_infos);
    ~ImageResource_VK();

  private:
    VkImage               image = VK_NULL_HANDLE;
    VkImageLayout         image_layout;
    VmaAllocation         memory;
};

class ImageViewResource_VK final
{
  public:
    ImageViewResource_VK(const std::string& name, const CI_TextureView& create_infos);
    ~ImageViewResource_VK();

  private:
    VkDescriptorImageInfo descriptor_infos;
    VkImageView           view;
};


class FenceResource_VK final
{
  public:
    FenceResource_VK(const std::string& name, const CI_Fence& create_infos);
    ~FenceResource_VK();

  private:
    VkFence fence = VK_NULL_HANDLE;
};

class SemaphoreResource_VK final
{
  public:
    SemaphoreResource_VK(const std::string& name, const CI_Semaphore& create_infos);
    ~SemaphoreResource_VK();

  private:
    VkSemaphore semaphore = VK_NULL_HANDLE;
};

} // namespace gfx::vulkan