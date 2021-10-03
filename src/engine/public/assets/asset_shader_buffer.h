#pragma once
#include "rendering/vulkan/shader_buffer.h"
#include "asset_base.h"

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <string>
#include <vulkan/vulkan.h>

class AShaderBuffer : public AssetBase
{
  public:
    AShaderBuffer(size_t data_size, VkBufferUsageFlags in_buffer_usage) : buffer_usage(in_buffer_usage)
    {
        resize_buffer(data_size < 16 ? 16 : data_size);
    }

    template <typename Struct_T> AShaderBuffer(const Struct_T& in_data, VkBufferUsageFlags in_buffer_usage) : buffer_usage(in_buffer_usage)
    {
        set_data<Struct_T>(in_data);
        if (data_size < 16)
            resize_buffer(16);
    }

    template <typename Struct_T> void set_data(const Struct_T& in_data)
    {
        resize_buffer(sizeof(Struct_T));
        write_buffer(&in_data, sizeof(in_data));
    }

    [[nodiscard]] size_t get_buffer_size() const
    {
        return data_size;
    }

    void resize_buffer(size_t in_data_size)
    {
        if (data_size != in_data_size)
        {
            data_size = in_data_size;
            data      = realloc(data, data_size);
        }
    }

    void write_buffer(const void* in_data, size_t in_size, size_t in_offset = 0)
    {
        if (in_size + in_offset > data_size)
        {
            LOG_WARNING("trying to write out of buffer range");
        }

        if (in_data)
            memcpy(static_cast<char*>(data) + in_offset, in_data, in_size);
    }

    [[nodiscard]] VkDescriptorBufferInfo* get_descriptor_buffer_info(uint32_t image_index);

  private:
    size_t                                             data_size = 0;
    void*                                              data      = nullptr;
    VkBufferUsageFlags                                 buffer_usage;
    std::vector<std::unique_ptr<ShaderBufferResource>> available_buffers;
};
