#pragma once

#include "gfx/resource/device.h"

#include <vulkan/vulkan.h>

namespace gfx::vulkan
{

namespace device
{
void create();
void destroy();

} // namespace device
VkDevice get_device();

class Device_VK : public Device
{
  public:
    Device_VK(uint8_t image_count) : Device(image_count)
    {
    }
    BufferHandle        create_buffer(const std::string& name, const CI_Buffer& create_infos) override;
    CommandBufferHandle create_command_buffer(const std::string& name, const CI_CommandBuffer& create_infos) override;
    BufferViewHandle create_buffer_view(const std::string& name, const CI_BufferView& create_infos) override
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
    TextureHandle create_texture(const std::string& name, const CI_Texture& create_infos) override
    {
        return GPU_NULL_HANDLE;
    }
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
};

} // namespace gfx::vulkan