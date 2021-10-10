#pragma once

#include "assets/asset_ptr.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "assets/asset_material.h"
#include "assets/asset_texture.h"
#include "backends/imgui_impl_glfw.h"
#include "rendering/graphics.h"
#include "rendering/renderer/render_pass_description.h"

class AMaterialInstance;
class ATexture2D;
class ATexture;

class DynamicBuffer final
{
  public:
    DynamicBuffer(VkBufferUsageFlags in_usage) : usage(in_usage)
    {
    }
    ~DynamicBuffer()
    {
        destroy();
    }

    void*                   acquire(size_t data_size);
    void                    release();
    [[nodiscard]] VkBuffer& get_buffer()
    {
        return buffer;
    }

  private:
    void destroy();

    VkBufferUsageFlags usage;

    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize   size   = 0;
    VkBuffer       buffer = VK_NULL_HANDLE;

    VkDeviceSize alignment_requirement = 256;

    void create_or_resize(size_t data_size);
};

class ImGuiImplementation
{
  public:
    ImGuiImplementation()
    {
        init_context();
        init_internal();
    }
    ~ImGuiImplementation();

    void ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, SwapchainFrame& render_context);

    TAssetPtr<ATexture2D>        font_image        = {};
    TAssetPtr<AMaterialInstance> material_instance = {};

    void init_context()
    {
        if (!context)
        {
            LOG_WARNING("CREATE");
            context = ImGui::CreateContext();
        }
        ImGui::SetCurrentContext(context);
     }

    [[nodiscard]] static RenderPassSettings get_ui_render_pass(const TAssetPtr<ATexture>& background_texture_buffer, std::unique_ptr<ImGuiImplementation>& imgui_implementation);

  private:
    void                         init_internal();
    ImGuiContext*                context           = nullptr;
    std::vector<DynamicBuffer>   vertex_buffer     = {};
    std::vector<DynamicBuffer>   index_buffer      = {};
};