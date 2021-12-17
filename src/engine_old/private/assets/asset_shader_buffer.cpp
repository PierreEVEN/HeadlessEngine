

#include "assets/asset_shader_buffer.h"

#include "engine_interface.h"
#include "rendering/vulkan/utils.h"

VkDescriptorBufferInfo* AShaderBuffer::get_descriptor_buffer_info(uint32_t image_index)
{
    if (image_index >= available_buffers.size())
    {
        for (int64_t i = available_buffers.size(); i <= image_index; ++i)
        {
            available_buffers.emplace_back(std::make_unique<ShaderBufferResource>(buffer_usage));
        }
    }

    available_buffers[image_index]->set_data(data, data_size);

    return available_buffers[image_index]->get_descriptor_buffer_info();
}
