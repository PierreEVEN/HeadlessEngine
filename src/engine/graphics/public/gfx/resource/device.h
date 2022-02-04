#pragma once
#include "resource_list.h"

#include <unordered_set>

namespace gfx
{
class Device
{
  public:
    static Device& get();

    template <typename Device_T, typename... Args_T> static void create_device(Args_T&&... args)
    {
        auto* device = new Device_T(std::forward<Args_T>(args)...);
        device->init();
    }

    static void destroy_device();
    virtual ~Device();
    
    virtual BufferHandle              create_buffer(const std::string& name, const CI_Buffer& create_infos)                             = 0;
    virtual BufferViewHandle          create_buffer_view(const std::string& name, const CI_BufferView& create_infos)                    = 0;
    virtual CommandBufferHandle       create_command_buffer(const std::string& name, const CI_CommandBuffer& create_infos)              = 0;
    virtual QueueHandle               create_queue(const std::string& name, const CI_Queue& create_infos)                               = 0;
    virtual SemaphoreHandle           create_semaphore(const std::string& name, const CI_Semaphore& create_infos)                       = 0;
    virtual FenceHandle               create_fence(const std::string& name, const CI_Fence& create_infos)                               = 0;
    virtual TextureHandle             create_texture(const std::string& name, const CI_Texture& create_infos)                           = 0;
    virtual TextureViewHandle         create_texture_view(const std::string& name, const CI_TextureView& create_infos)                  = 0;
    virtual ShaderHandle              create_shader(const std::string& name, const CI_Shader& create_infos)                             = 0;
    virtual SamplerHandle             create_sampler(const std::string& name, const CI_Sampler& create_infos)                           = 0;
    virtual PipelineLayoutHandle      create_pipeline_layout(const std::string& name, const CI_PipelineLayout& create_infos)            = 0;
    virtual RenderPassHandle          create_render_pass(const std::string& name, const CI_RenderPass& create_infos)                    = 0;
    virtual PipelineHandle            create_pipeline(const std::string& name, const CI_Pipeline& create_infos)                         = 0;
    virtual DescriptorSetLayoutHandle create_descriptor_set_layout(const std::string& name, const CI_DescriptorSetLayout& create_infos) = 0;
    virtual FramebufferHandle         create_framebuffer(const std::string& name, const CI_Framebuffer& create_infos)                   = 0;
    virtual DescriptorPoolHandle      create_descriptor_pool(const std::string& name, const CI_DescriptorPool& create_infos)            = 0;
    virtual CommandPoolHandle         create_command_pool(const std::string& name, const CI_CommandPool& create_infos)                  = 0;
    virtual SurfaceHandle             create_surface(const std::string& name, const CI_Surface& create_infos)                           = 0;
    virtual SwapchainHandle           create_swapchain(const std::string& name, const CI_Swapchain& create_infos)                       = 0;

    void acquire_resource(ResourceHandle resource)
    {
        acquired_resources[current_frame_id].emplace_back(resource);
    }

    void release_frame(uint8_t frame);

    void set_frame(uint8_t frame_id);

    [[nodiscard]] uint8_t get_current_frame() const
    {
        return current_frame_id;
    }
    [[nodiscard]] uint8_t get_frame_count() const
    {
        return frame_count;
    }

    void         register_resource(ResourceHandle handle);
    virtual void wait_device() = 0;

  protected:
    Device(uint8_t image_count);
    virtual void init() = 0;
    void         free_allocations();
  private:
    friend class IGpuResource;
    void destroy_resource(ResourceHandle resource_handle);

    std::unordered_set<ResourceHandle>       resources;
    std::vector<std::vector<ResourceHandle>> acquired_resources;

    uint8_t current_frame_id;
    uint8_t frame_count;
};
} // namespace gfx