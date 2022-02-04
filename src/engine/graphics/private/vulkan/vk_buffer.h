#pragma once

#include "gfx/buffer.h"
#include "gfx/resource/resource_list.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "vulkan/vk_unit.h"

namespace gfx::vulkan
{
class Buffer_VK final : public Buffer
{
  public:
    Buffer_VK(const std::string& buffer_name, uint32_t buffer_stride, uint32_t element_count, EBufferUsage buffer_usage, EBufferAccess in_buffer_access, EBufferType buffer_type);
    ~Buffer_VK() override;

    void bind_buffer(VkCommandBuffer command_buffer);

    [[nodiscard]] const VkDescriptorBufferInfo& get_buffer_infos() const
    {
        return frame_data->buffer_infos;
    }

    void resize(uint32_t element_count) override;

  protected:
    void* acquire_data_ptr() override;
    void  submit_data() override;

  private:
    struct FrameData
    {
        VkDescriptorBufferInfo buffer_infos             = {};
        BufferHandle           buffer                   = GPU_NULL_HANDLE;
        bool                   dirty                    = false;
        uint32_t               allocated_count          = 0;
        uint32_t               previous_allocated_count = 0;
    };

    uint32_t                          allocated_count = 0;
    uint8_t*                          dynamic_data    = nullptr;
    SwapchainImageResource<FrameData> frame_data      = {};

    void create_or_recreate_buffer(FrameData& frame_data);
    void resize_current();
};

} // namespace gfx::vulkan