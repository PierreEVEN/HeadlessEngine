
#include "vk_command_buffer.h"

#include "vk_helper.h"
#include "vulkan/vk_command_pool.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
namespace gfx::vulkan
{
CommandBuffer_VK::CommandBuffer_VK(const std::string& name)
{
    const VkCommandBufferAllocateInfo command_buffer_infos{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = command_pool::get(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    uint8_t cmd = 0;
    for (auto& buffer : command_buffer)
    {
        VK_CHECK(vkAllocateCommandBuffers(get_device(), &command_buffer_infos, &buffer), "Failed to allocate command buffer");
        debug_set_object_name(stringutils::format("command buffer %s #%d", name.c_str(), cmd++), buffer);
    }
}

CommandBuffer_VK::~CommandBuffer_VK()
{
    vkDeviceWaitIdle(get_device());
    for (auto& buffer : command_buffer)
        vkFreeCommandBuffers(get_device(), command_pool::get(), 1, &buffer);
}

void CommandBuffer_VK::draw_mesh(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer)
{
    (void)in_buffer;
    (void)in_material;
    (void)render_layer;
}

void CommandBuffer_VK::draw_mesh_indirect(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer)
{
    (void)in_buffer;
    (void)in_material;
    (void)render_layer;
}

void CommandBuffer_VK::draw_mesh_instanced(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer)
{
    (void)in_buffer;
    (void)in_material;
    (void)render_layer;
}

void CommandBuffer_VK::draw_mesh_instanced_indirect(Mesh* in_buffer, MaterialInterface* in_material, RenderLayer render_layer)
{
    (void)in_buffer;
    (void)in_material;
    (void)render_layer;
}
} // namespace gfx::vulkan