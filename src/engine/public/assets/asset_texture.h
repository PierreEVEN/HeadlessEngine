#pragma once
#include "asset_base.h"
#include "imgui.h"
#include "rendering/renderer/swapchain_image_resource.h"
#include "rendering/shaders/shader_property.h"

class ATexture : public AssetBase
{
  public:
    virtual ~ATexture();

    [[nodiscard]] VkDescriptorImageInfo* get_descriptor_image_info(uint32_t image_index);
    [[nodiscard]] virtual VkImage        get_image(uint32_t image_index) const = 0;
    [[nodiscard]] virtual VkImageView    get_view(uint32_t image_index) const  = 0;
    [[nodiscard]] virtual VkImageLayout  get_image_layout(uint32_t image_index) const
    {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    [[nodiscard]] virtual VkSampler get_sampler(uint32_t image_index = 0) const
    {
        return VK_NULL_HANDLE;
    }

    ImTextureID get_imgui_handle(uint32_t image_index, VkDescriptorSetLayout descriptor_set_layout);

  protected:
    void mark_descriptor_dirty()
    {
        for (size_t i = 0; i < descriptor_image_infos.get_max_instance_count(); ++i)
            descriptor_image_infos[i].is_dirty = true;

        for (size_t i = 0; i < imgui_desc_set.get_max_instance_count(); ++i)
            imgui_desc_set[i].is_dirty = true;
    }

private:

    struct DescriptorState
    {
        bool                  is_dirty = true;
        VkDescriptorImageInfo descriptor;
    };

    struct ImGuiDescriptorState
    {
        bool            is_dirty = true;
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
    };
    SwapchainImageResource<DescriptorState>      descriptor_image_infos = {};
    SwapchainImageResource<ImGuiDescriptorState> imgui_desc_set         = {};
};

class ATexture2D : public ATexture
{
  public:
    ATexture2D(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint8_t in_channels);
    virtual ~ATexture2D();

    [[nodiscard]] VkImage get_image(uint32_t image_index = 0) const override
    {
        return image;
    }
    [[nodiscard]] VkImageView get_view(uint32_t image_index = 0) const override
    {
        return view;
    }
    [[nodiscard]] VkSampler get_sampler(uint32_t image_index = 0) const override
    {
        return sampler;
    }

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
};
