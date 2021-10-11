

#include "rendering/renderer/render_pass_description.h"

VkFormat RenderPassSettings::get_resolve_format() const
{
    for (const auto& attachment : color_attachments)
        return attachment.image_format;
    return VK_FORMAT_UNDEFINED;
}

bool RenderPassSettings::has_resolve_attachment() const
{
    bool has_color_attachment = false;
    for (const auto& attachment : color_attachments)
        has_color_attachment = true;

    return sample_count != VK_SAMPLE_COUNT_1_BIT && has_color_attachment;
}

std::vector<VkClearValue> RenderPassSettings::get_clear_values() const
{
    std::vector<VkClearValue> clear_values;
    for (const auto& attachment : color_attachments)
    {
        if (attachment.clear_value)
            clear_values.emplace_back(attachment.clear_value.value());
    }
    if (depth_attachment && depth_attachment->clear_value)
        clear_values.emplace_back(depth_attachment->clear_value.value());

    return clear_values;
}

uint32_t RenderPassSettings::get_total_attachment_count() const
{
    return static_cast<uint32_t>(color_attachments.size()) + (depth_attachment ? 1 : 0) + (has_resolve_attachment() ? 1 : 0);
}
