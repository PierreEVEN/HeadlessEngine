#pragma once
#include "asset_base.h"

#include <vulkan/vulkan.hpp>

class ATexture : public AssetBase
{
  public:
    ATexture(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint8_t in_channels);
    virtual ~ATexture();
    
    VkDescriptorImageInfo* get_descriptor_image_info(uint32_t image_index);

  private:
    void create_image_and_view(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint8_t in_channels);
    void create_sampler();

    VkImage        image       = VK_NULL_HANDLE;
    VkDeviceMemory memory      = VK_NULL_HANDLE;
    VkImageView    view        = VK_NULL_HANDLE;
    VkSampler      sampler     = VK_NULL_HANDLE;
    VkFormat       format      = VK_FORMAT_UNDEFINED;
    VkDeviceSize   data_size   = 0;
    uint32_t       mips_levels = 0;
    uint8_t        channels    = 0;

    std::vector<VkDescriptorImageInfo> descriptor_image_infos = {};
};
