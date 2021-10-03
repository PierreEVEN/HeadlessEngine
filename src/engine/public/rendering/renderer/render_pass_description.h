#pragma once

#include "assets/asset_ptr.h"

#include <cpputils/eventmanager.hpp>
#include <optional>
#include <vulkan/vulkan.hpp>

class ATexture2D;
class AShaderBuffer;
class AShader;
struct SwapchainFrame;

DECLARE_DELEGATE_MULTICAST(EventRenderRenderPass, SwapchainFrame*)

class AMaterialInstance;
class NCamera;

class DrawInterface;

struct SwapchainFrame
{
    bool            is_valid           = false;
    VkCommandBuffer command_buffer     = VK_NULL_HANDLE;
    VkFramebuffer   framebuffer        = VK_NULL_HANDLE;
    uint32_t        image_index        = 0;
    uint32_t        res_x              = 0;
    uint32_t        res_y              = 0;
    AMaterialInstance* last_used_material = nullptr;
    NCamera*        view               = nullptr;
    std::string     render_pass        = "";
};

struct RenderPassAttachment
{
    VkFormat                    image_format = VK_FORMAT_UNDEFINED;
    std::optional<VkClearValue> clear_value;
};

struct RenderPassSettings
{
    std::string                         pass_name;
    VkSampleCountFlagBits               sample_count          = VK_SAMPLE_COUNT_1_BIT;
    bool                                b_use_swapchain_image = false;
    EventRenderRenderPass               on_pass_rendering;
    std::vector<RenderPassAttachment>   color_attachments;
    std::optional<RenderPassAttachment> depth_attachment;

    [[nodiscard]] VkFormat                  get_resolve_format() const;
    [[nodiscard]] bool                      has_resolve_attachment() const;
    [[nodiscard]] std::vector<VkClearValue> get_clear_values() const;
    [[nodiscard]] uint32_t                  get_total_attachment_count() const;
};