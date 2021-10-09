
#include "ui/imgui/imgui_impl_vulkan.h"
#include "assets/asset_base.h"
#include "assets/asset_material.h"
#include "assets/asset_material_instance.h"
#include "assets/asset_shader.h"
#include "imgui.h"

#include "assets/asset_texture.h"
#include <cpputils/logger.hpp>

#include "backends/imgui_impl_glfw.h"
#include "rendering/graphics.h"
#include "rendering/renderer/render_pass.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/descriptor_pool.h"
#include "rendering/vulkan/material_pipeline.h"

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant_value) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static std::vector<uint32_t> __glsl_shader_vert_spv = {
    0x07230203, 0x00010000, 0x00080001, 0x0000002e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x0000000b, 0x0000000f, 0x00000015, 0x0000001b, 0x0000001c, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00030005, 0x00000009, 0x00000000,
    0x00050006, 0x00000009, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x00000009, 0x00000001, 0x00005655, 0x00030005, 0x0000000b, 0x0074754f, 0x00040005, 0x0000000f, 0x6c6f4361, 0x0000726f, 0x00030005, 0x00000015,
    0x00565561, 0x00060005, 0x00000019, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x00000019, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x0000001b, 0x00000000, 0x00040005, 0x0000001c,
    0x736f5061, 0x00000000, 0x00060005, 0x0000001e, 0x73755075, 0x6e6f4368, 0x6e617473, 0x00000074, 0x00050006, 0x0000001e, 0x00000000, 0x61635375, 0x0000656c, 0x00060006, 0x0000001e, 0x00000001, 0x61725475, 0x616c736e,
    0x00006574, 0x00030005, 0x00000020, 0x00006370, 0x00040047, 0x0000000b, 0x0000001e, 0x00000000, 0x00040047, 0x0000000f, 0x0000001e, 0x00000002, 0x00040047, 0x00000015, 0x0000001e, 0x00000001, 0x00050048, 0x00000019,
    0x00000000, 0x0000000b, 0x00000000, 0x00030047, 0x00000019, 0x00000002, 0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00050048, 0x0000001e, 0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001,
    0x00000023, 0x00000008, 0x00030047, 0x0000001e, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040017,
    0x00000008, 0x00000006, 0x00000002, 0x0004001e, 0x00000009, 0x00000007, 0x00000008, 0x00040020, 0x0000000a, 0x00000003, 0x00000009, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000003, 0x00040015, 0x0000000c, 0x00000020,
    0x00000001, 0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020, 0x0000000e, 0x00000001, 0x00000007, 0x0004003b, 0x0000000e, 0x0000000f, 0x00000001, 0x00040020, 0x00000011, 0x00000003, 0x00000007, 0x0004002b,
    0x0000000c, 0x00000013, 0x00000001, 0x00040020, 0x00000014, 0x00000001, 0x00000008, 0x0004003b, 0x00000014, 0x00000015, 0x00000001, 0x00040020, 0x00000017, 0x00000003, 0x00000008, 0x0003001e, 0x00000019, 0x00000007,
    0x00040020, 0x0000001a, 0x00000003, 0x00000019, 0x0004003b, 0x0000001a, 0x0000001b, 0x00000003, 0x0004003b, 0x00000014, 0x0000001c, 0x00000001, 0x0004001e, 0x0000001e, 0x00000008, 0x00000008, 0x00040020, 0x0000001f,
    0x00000009, 0x0000001e, 0x0004003b, 0x0000001f, 0x00000020, 0x00000009, 0x00040020, 0x00000021, 0x00000009, 0x00000008, 0x0004002b, 0x00000006, 0x00000028, 0x00000000, 0x0004002b, 0x00000006, 0x00000029, 0x3f800000,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007, 0x00000010, 0x0000000f, 0x00050041, 0x00000011, 0x00000012, 0x0000000b, 0x0000000d, 0x0003003e, 0x00000012,
    0x00000010, 0x0004003d, 0x00000008, 0x00000016, 0x00000015, 0x00050041, 0x00000017, 0x00000018, 0x0000000b, 0x00000013, 0x0003003e, 0x00000018, 0x00000016, 0x0004003d, 0x00000008, 0x0000001d, 0x0000001c, 0x00050041,
    0x00000021, 0x00000022, 0x00000020, 0x0000000d, 0x0004003d, 0x00000008, 0x00000023, 0x00000022, 0x00050085, 0x00000008, 0x00000024, 0x0000001d, 0x00000023, 0x00050041, 0x00000021, 0x00000025, 0x00000020, 0x00000013,
    0x0004003d, 0x00000008, 0x00000026, 0x00000025, 0x00050081, 0x00000008, 0x00000027, 0x00000024, 0x00000026, 0x00050051, 0x00000006, 0x0000002a, 0x00000027, 0x00000000, 0x00050051, 0x00000006, 0x0000002b, 0x00000027,
    0x00000001, 0x00070050, 0x00000007, 0x0000002c, 0x0000002a, 0x0000002b, 0x00000028, 0x00000029, 0x00050041, 0x00000011, 0x0000002d, 0x0000001b, 0x0000000d, 0x0003003e, 0x0000002d, 0x0000002c, 0x000100fd, 0x00010038};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
    fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static std::vector<uint32_t> __glsl_shader_frag_spv = {
    0x07230203, 0x00010000, 0x00080001, 0x0000001e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00040005, 0x00000009, 0x6c6f4366,
    0x0000726f, 0x00030005, 0x0000000b, 0x00000000, 0x00050006, 0x0000000b, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x0000000b, 0x00000001, 0x00005655, 0x00030005, 0x0000000d, 0x00006e49, 0x00050005, 0x00000016,
    0x78655473, 0x65727574, 0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x0000001e, 0x00000000, 0x00040047, 0x00000016, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x00000021,
    0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b,
    0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006, 0x00000002, 0x0004001e, 0x0000000b, 0x00000007, 0x0000000a, 0x00040020, 0x0000000c, 0x00000001, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
    0x00000001, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040020, 0x00000010, 0x00000001, 0x00000007, 0x00090019, 0x00000013, 0x00000006, 0x00000001, 0x00000000,
    0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x00000014, 0x00000013, 0x00040020, 0x00000015, 0x00000000, 0x00000014, 0x0004003b, 0x00000015, 0x00000016, 0x00000000, 0x0004002b, 0x0000000e, 0x00000018,
    0x00000001, 0x00040020, 0x00000019, 0x00000001, 0x0000000a, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x00000010, 0x00000011, 0x0000000d, 0x0000000f, 0x0004003d,
    0x00000007, 0x00000012, 0x00000011, 0x0004003d, 0x00000014, 0x00000017, 0x00000016, 0x00050041, 0x00000019, 0x0000001a, 0x0000000d, 0x00000018, 0x0004003d, 0x0000000a, 0x0000001b, 0x0000001a, 0x00050057, 0x00000007,
    0x0000001c, 0x00000017, 0x0000001b, 0x00050085, 0x00000007, 0x0000001d, 0x00000012, 0x0000001c, 0x0003003e, 0x00000009, 0x0000001d, 0x000100fd, 0x00010038};

void* DynamicBuffer::acquire(size_t data_size)
{
    if (data_size > size || data_size < size * 0.5)
        create_or_resize(static_cast<size_t>(data_size * 1.5));

    void* buffer_ptr = nullptr;
    VK_ENSURE(vkMapMemory(Graphics::get()->get_logical_device(), memory, 0, data_size, 0, &buffer_ptr), "failed to map buffer memory");
    return buffer_ptr;
}

void DynamicBuffer::release()
{
    vkUnmapMemory(Graphics::get()->get_logical_device(), memory);
}

void DynamicBuffer::destroy()
{
    if (buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(Graphics::get()->get_logical_device(), buffer, vulkan_common::allocation_callback);

    if (memory != VK_NULL_HANDLE)
        vkFreeMemory(Graphics::get()->get_logical_device(), memory, vulkan_common::allocation_callback);
}

void DynamicBuffer::create_or_resize(size_t data_size)
{
    destroy();

    const VkDeviceSize vertex_buffer_size_aligned = ((data_size - 1) / alignment_requirement + 1) * alignment_requirement;

    VkBufferCreateInfo buffer_info{
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = vertex_buffer_size_aligned,
        .usage       = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VK_ENSURE(vkCreateBuffer(Graphics::get()->get_logical_device(), &buffer_info, vulkan_common::allocation_callback, &buffer), "failed to create buffer");

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(Graphics::get()->get_logical_device(), buffer, &requirements);
    alignment_requirement = (alignment_requirement > requirements.alignment) ? alignment_requirement : requirements.alignment;
    VkMemoryAllocateInfo alloc_info{
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize  = requirements.size,
        .memoryTypeIndex = vulkan_utils::find_memory_type(Graphics::get()->get_physical_device(), requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
    };
    VK_ENSURE(vkAllocateMemory(Graphics::get()->get_logical_device(), &alloc_info, vulkan_common::allocation_callback, &memory), "failed to allocate memory");

    VK_ENSURE(vkBindBufferMemory(Graphics::get()->get_logical_device(), buffer, memory, 0), "failed to bind memory");

    size = data_size;
}

ImGuiImplementation::ImGuiImplementation()
{
    // Init imgui
    LOG_INFO("[ Imgui] Initialize imgui ressources");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(ImGui::CreateContext());
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGuiStyle& style        = ImGui::GetStyle();
    style.WindowRounding     = 0;
    style.ScrollbarRounding  = 0;
    style.TabRounding        = 0;
    style.WindowBorderSize   = 1;
    style.PopupBorderSize    = 1;
    style.WindowTitleAlign.x = 0.5f;
    style.FramePadding.x     = 6.f;
    style.FramePadding.y     = 6.f;
    style.WindowPadding.x    = 4.f;
    style.WindowPadding.y    = 4.f;
    style.GrabMinSize        = 16.f;
    style.ScrollbarSize      = 20.f;
    style.IndentSpacing      = 30.f;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    //@TODO handle imgui font
    // G_IMGUI_DEFAULT_FONT = io.Fonts->AddFontFromFileTTF(G_DEFAULT_FONT_PATH.GetValue().GetData(), 20.f);

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(Graphics::get()->get_glfw_handle(), true);

    // Create vertex buffers
    for (uint32_t i = 0; i < Graphics::get()->get_swapchain_config()->get_image_count(); ++i)
    {
        vertex_buffer.emplace_back(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        index_buffer.emplace_back(VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    // Create font texture
    uint8_t* pixels;
    int      width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    font_image = AssetManager::get()->create<ATexture2D>("imgui_default_font_texture", std::vector<uint8_t>(pixels, pixels + width * height), width, height, 1);

    // Create material
    const TAssetPtr<AShader> vertex_shader = AssetManager::get()->create<AShader>("imgui_vertex_shader", __glsl_shader_vert_spv,
                                                                                  ShaderInfos{
                                                                                      .shader_stage = VK_SHADER_STAGE_VERTEX_BIT,
                                                                                  });

    const TAssetPtr<AShader> fragment_shader = AssetManager::get()->create<AShader>("imgui_fragment_shader", __glsl_shader_frag_spv,
                                                                                    ShaderInfos{
                                                                                        .shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                                                        .textures{
                                                                                            TextureProperty{.binding_name = "sTexture", .texture = TAssetPtr<ATexture>("imgui_font_texture")},
                                                                                        },
                                                                                    });

    const TAssetPtr<AMaterialBase> material_base = AssetManager::get()->create<AMaterialBase>("imgui_base_material", MaterialInfos{
                                                                                                                         .vertex_stage    = vertex_shader,
                                                                                                                         .fragment_stage  = fragment_shader,
                                                                                                                         .renderer_passes = {"post_processing_0"},
                                                                                                                     });

    material_instance = AssetManager::get()->create<AMaterialInstance>("imgui_material", material_base);
}

ImGuiImplementation::~ImGuiImplementation()
{
    ImGui_ImplGlfw_Shutdown();
}

void ImGuiImplementation::ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, SwapchainFrame& render_context)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    const int fb_width  = static_cast<int>(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    const int fb_height = static_cast<int>(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0 || draw_data->TotalVtxCount == 0)
        return;

    const uint32_t    image_index = render_context.image_index;
    const std::string render_pass = render_context.render_pass;

    const size_t vertex_data_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
    const size_t index_data_size  = draw_data->TotalIdxCount * sizeof(ImDrawVert);

    /**
     * BUILD VERTEX BUFFERS
     */
    // Get pointer to buffer data
    ImDrawVert* vertex_data = static_cast<ImDrawVert*>(vertex_buffer[image_index].acquire(vertex_data_size));
    ImDrawIdx*  index_data  = static_cast<ImDrawIdx*>(index_buffer[image_index].acquire(index_data_size));

    // copy data
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        memcpy(vertex_data, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(index_data, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vertex_data += cmd_list->VtxBuffer.Size;
        index_data += cmd_list->IdxBuffer.Size;
    }

    vertex_buffer[image_index].release();
    index_buffer[image_index].release();

    /**
     * PREPARE MATERIALS
     */

    auto pipeline = material_instance->get_material_base()->get_pipeline(render_pass);
    vkCmdBindPipeline(render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());
    vkCmdBindVertexBuffers(render_context.command_buffer, 0, 1, &vertex_buffer[image_index].get_buffer(), 0);
    vkCmdBindIndexBuffer(render_context.command_buffer, index_buffer[image_index].get_buffer(), 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

    float scale[2] = {
        2.0f / draw_data->DisplaySize.x,
        2.0f / draw_data->DisplaySize.y,
    };
    float translate[2] = {
        -1.0f - draw_data->DisplayPos.x * scale[0],
        -1.0f - draw_data->DisplayPos.y * scale[1],
    };

    vkCmdPushConstants(render_context.command_buffer, *pipeline->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2, scale);
    vkCmdPushConstants(render_context.command_buffer, *pipeline->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);

    /**
     * Draw meshs
     */

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
                pcmd->UserCallback(cmd_list, pcmd);
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Negative offsets are illegal for vkCmdSetScissor
                    if (clip_rect.x < 0.0f)
                        clip_rect.x = 0.0f;
                    if (clip_rect.y < 0.0f)
                        clip_rect.y = 0.0f;

                    // Apply scissor/clipping rectangle
                    VkRect2D scissor;
                    scissor.offset.x      = static_cast<int32_t>(clip_rect.x);
                    scissor.offset.y      = static_cast<int32_t>(clip_rect.y);
                    scissor.extent.width  = static_cast<uint32_t>(clip_rect.z - clip_rect.x);
                    scissor.extent.height = static_cast<uint32_t>(clip_rect.w - clip_rect.y);
                    vkCmdSetScissor(render_context.command_buffer, 0, 1, &scissor);

                    // Bind descriptorset with font or user texture
                    VkDescriptorSet desc_set[1] = {static_cast<VkDescriptorSet>(pcmd->TextureId)};
                    vkCmdBindDescriptorSets(render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->get_pipeline_layout(), 0, 1, desc_set, 0, NULL);

                    // Draw
                    vkCmdDrawIndexed(render_context.command_buffer, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
                }
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

/*
bool ImGuiInstance::ImGui_ImplVulkan_CreateDeviceObjects()
{
    VkResult err;

    TAssetPtr<AShader> vertex_shader =
        nullptr; // AssetManager::get()->create<AShader>("imgui_vertex_shader", std::vector<uint32_t>(__glsl_shader_vert_spv, __glsl_shader_vert_spv + sizeof(__glsl_shader_vert_spv)), EShaderStage::VERTEX_SHADER);
    TAssetPtr<AShader> fragment_shader = nullptr; //        AssetManager::get()->create<AShader>("imgui_fragment_shader", std::vector<uint32_t>(__glsl_shader_frag_spv, __glsl_shader_frag_spv +
                                                  //        sizeof(__glsl_shader_frag_spv)), EShaderStage::FRAGMENT_SHADER);
    LOG_FATAL("@TODO update shader creation");

    if (!g_FontSampler)
    {
        VkSamplerCreateInfo info = {};
        info.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter           = VK_FILTER_LINEAR;
        info.minFilter           = VK_FILTER_LINEAR;
        info.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.minLod              = -1000;
        info.maxLod              = 1000;
        info.maxAnisotropy       = 1.0f;
        err                      = vkCreateSampler(Graphics::get()->get_logical_device(), &info, vulkan_common::allocation_callback, &g_FontSampler);
        VK_ENSURE(err);
    }

    if (!g_DescriptorSetLayout)
    {
        VkSampler                    sampler[1] = {g_FontSampler};
        VkDescriptorSetLayoutBinding binding[1] = {};
        binding[0].descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding[0].descriptorCount              = 1;
        binding[0].stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutCreateInfo info    = {};
        info.sType                              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount                       = 1;
        info.pBindings                          = binding;
        err                                     = vkCreateDescriptorSetLayout(Graphics::get()->get_logical_device(), &info, vulkan_common::allocation_callback, &g_DescriptorSetLayout);
        VK_ENSURE(err);
    }

    if (!g_PipelineLayout)
    {
        // Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
        VkPushConstantRange push_constants[1]    = {};
        push_constants[0].stageFlags             = VK_SHADER_STAGE_VERTEX_BIT;
        push_constants[0].offset                 = sizeof(float) * 0;
        push_constants[0].size                   = sizeof(float) * 4;
        VkDescriptorSetLayout      set_layout[1] = {g_DescriptorSetLayout};
        VkPipelineLayoutCreateInfo layout_info   = {};
        layout_info.sType                        = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_info.setLayoutCount               = 1;
        layout_info.pSetLayouts                  = set_layout;
        layout_info.pushConstantRangeCount       = 1;
        layout_info.pPushConstantRanges          = push_constants;
        err                                      = vkCreatePipelineLayout(Graphics::get()->get_logical_device(), &layout_info, vulkan_common::allocation_callback, &g_PipelineLayout);
        VK_ENSURE(err);
    }

    VkPipelineShaderStageCreateInfo stage[2] = {};
    stage[0].sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage[0].stage                           = VK_SHADER_STAGE_VERTEX_BIT;
    stage[0].module                          = vertex_shader->get_shader_module();
    stage[0].pName                           = "main";
    stage[1].sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage[1].stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage[1].module                          = fragment_shader->get_shader_module();
    stage[1].pName                           = "main";

    VkVertexInputBindingDescription binding_desc[1] = {};
    binding_desc[0].stride                          = sizeof(ImDrawVert);
    binding_desc[0].inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attribute_desc[3] = {};
    attribute_desc[0].location                          = 0;
    attribute_desc[0].binding                           = binding_desc[0].binding;
    attribute_desc[0].format                            = VK_FORMAT_R32G32_SFLOAT;
    attribute_desc[0].offset                            = IM_OFFSETOF(ImDrawVert, pos);
    attribute_desc[1].location                          = 1;
    attribute_desc[1].binding                           = binding_desc[0].binding;
    attribute_desc[1].format                            = VK_FORMAT_R32G32_SFLOAT;
    attribute_desc[1].offset                            = IM_OFFSETOF(ImDrawVert, uv);
    attribute_desc[2].location                          = 2;
    attribute_desc[2].binding                           = binding_desc[0].binding;
    attribute_desc[2].format                            = VK_FORMAT_R8G8B8A8_UNORM;
    attribute_desc[2].offset                            = IM_OFFSETOF(ImDrawVert, col);

    VkPipelineVertexInputStateCreateInfo vertex_info = {};
    vertex_info.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_info.vertexBindingDescriptionCount        = 1;
    vertex_info.pVertexBindingDescriptions           = binding_desc;
    vertex_info.vertexAttributeDescriptionCount      = 3;
    vertex_info.pVertexAttributeDescriptions         = attribute_desc;

    VkPipelineInputAssemblyStateCreateInfo ia_info = {};
    ia_info.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia_info.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewport_info = {};
    viewport_info.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount                     = 1;
    viewport_info.scissorCount                      = 1;

    VkPipelineRasterizationStateCreateInfo raster_info = {};
    raster_info.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_info.polygonMode                            = VK_POLYGON_MODE_FILL;
    raster_info.cullMode                               = VK_CULL_MODE_NONE;
    raster_info.frontFace                              = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_info.lineWidth                              = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms_info = {};
    ms_info.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    if (vulkan_common::get_msaa_sample_count() != 0)
        ms_info.rasterizationSamples = static_cast<VkSampleCountFlagBits>(vulkan_common::get_msaa_sample_count());
    else
        ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_attachment[1] = {};
    color_attachment[0].blendEnable                         = VK_TRUE;
    color_attachment[0].srcColorBlendFactor                 = VK_BLEND_FACTOR_SRC_ALPHA;
    color_attachment[0].dstColorBlendFactor                 = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment[0].colorBlendOp                        = VK_BLEND_OP_ADD;
    color_attachment[0].srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment[0].dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    color_attachment[0].alphaBlendOp                        = VK_BLEND_OP_ADD;
    color_attachment[0].colorWriteMask                      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_info = {};
    depth_info.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    VkPipelineColorBlendStateCreateInfo blend_info = {};
    blend_info.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_info.attachmentCount                     = 1;
    blend_info.pAttachments                        = color_attachment;

    VkDynamicState                   dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state     = {};
    dynamic_state.sType                                = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount                    = (uint32_t)IM_ARRAYSIZE(dynamic_states);
    dynamic_state.pDynamicStates                       = dynamic_states;

    VkGraphicsPipelineCreateInfo info = {};
    info.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.flags                        = g_PipelineCreateFlags;
    info.stageCount                   = 2;
    info.pStages                      = stage;
    info.pVertexInputState            = &vertex_info;
    info.pInputAssemblyState          = &ia_info;
    info.pViewportState               = &viewport_info;
    info.pRasterizationState          = &raster_info;
    info.pMultisampleState            = &ms_info;
    info.pDepthStencilState           = &depth_info;
    info.pColorBlendState             = &blend_info;
    info.pDynamicState                = &dynamic_state;
    info.layout                       = g_PipelineLayout;
    info.renderPass                   = Graphics::get()->get_renderer()->get_render_pass("stage_ui")->get_render_pass();
    err                               = vkCreateGraphicsPipelines(Graphics::get()->get_logical_device(), g_pipeline_cache, 1, &info, vulkan_common::allocation_callback, &g_Pipeline);
    VK_ENSURE(err);

    return true;
}
*/