#pragma once

#include "gfx/buffer.h"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "vulkan/vk_unit.h"

namespace gfx::vulkan
{
class Buffer_VK final : public Buffer
{
  public:
    Buffer_VK(const std::string& buffer_name, uint32_t buffer_stride, uint32_t elements, EBufferUsage buffer_usage, EBufferAccess in_buffer_access, EBufferType buffer_type);
    ~Buffer_VK() override;

    void                          set_data(const void* data, size_t data_length, size_t offset) override;
    void                          bind_buffer(VkCommandBuffer command_buffer);
    const VkDescriptorBufferInfo& get_buffer_infos() const
    {
        return frame_data->buffer_infos;
    }

  protected:
    void* get_ptr() override;
    void  submit_data() override;

  private:
    struct FrameData
    {
        VkDescriptorBufferInfo buffer_infos;
        VkBuffer               buffer;
        VmaAllocation          memory;
    };

    SwapchainImageResource<FrameData> frame_data;
};

} // namespace gfx::vulkan