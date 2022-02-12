#include "vulkan/vk_texture.h"

#include "vk_types.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_one_time_command_buffer.h"

namespace gfx::vulkan
{
Texture_VK::Texture_VK(const std::string& name, uint32_t pixel_width, uint32_t pixel_height, uint32_t pixel_depth, const TextureParameter& parameters) : Texture(name, pixel_width, pixel_height, pixel_depth, parameters)
{
    if (parameters.read_only)
    {
        images = SwapchainImageResource<TGpuHandle<ImageResource_VK>>::make_static();
        views  = SwapchainImageResource<TGpuHandle<ImageViewResource_VK>>::make_static();
    }

    for (uint8_t i = 0; i < images.get_max_instance_count(); ++i)
    {
        images[i] = TGpuHandle<ImageResource_VK>(stringutils::format("image:%s", name.c_str()), ImageResource_VK::CI_Texture{
                                                                      .width              = width,
                                                                      .height             = height,
                                                                      .depth              = depth,
                                                                      .texture_parameters = parameters,
                                                                  });

        views[i] = TGpuHandle<ImageViewResource_VK>(stringutils::format("view:%s", name.c_str()), ImageViewResource_VK::CI_TextureView{
                                                                        .texture_parameters = parameters,
                                                                        .used_image         = images[i],
                                                                    });
    }
}

Texture_VK::Texture_VK(const std::string& name, uint32_t image_width, uint32_t image_height, uint32_t image_depth, const TextureParameter& parameters, SwapchainImageResource<VkImage>& existing_images)
    : Texture(name, image_width, image_height, image_depth, parameters)
{
    for (uint8_t i = 0; i < existing_images.get_max_instance_count(); ++i)
    {
        const auto framebuffer_image = TGpuHandle<ImageResource_VK>(stringutils::format("image_external:%s", name.c_str()),
                                                                    ImageResource_VK::CI_Texture{
                                                                        .width              = width,
                                                                        .height             = height,
                                                                        .depth              = depth,
                                                                        .texture_parameters = parameters,
                                                                    },
                                                                    existing_images[i]);
        
        views[i] = TGpuHandle<ImageViewResource_VK>(stringutils::format("view:%s", name.c_str()), ImageViewResource_VK::CI_TextureView{
                                                                        .texture_parameters = parameters,
                                                                        .used_image         = framebuffer_image,
                                                                    });
    }
}

void Texture_VK::set_pixels(const std::vector<uint8_t>& data)
{
    if (auto& image = *images)
        image->set_pixels(data);
}

} // namespace gfx::vulkan
