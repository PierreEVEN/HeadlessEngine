
#include "rendering/renderer/surface.h"

#include <array>
#include <mutex>
#include <unordered_map>

#include "backends/imgui_impl_glfw.h"
#include "rendering/vulkan/command_pool.h"
#include "rendering/vulkan/descriptor_pool.h"
#include "rendering/vulkan/framebuffer.h"
#include "ui/imgui/imgui_impl_vulkan.h"
#include "ui/window/window_base.h"
#include "ui/window/windows/profiler.h"
#include "rendering/gfx_context.h"
#include "rendering/renderer/render_pass.h"

#include <cpputils/logger.hpp>

std::mutex                                window_map_lock;
std::unordered_map<GLFWwindow*, Surface*> window_map;

void framebuffer_size_callback(GLFWwindow* handle, const int res_x, const int res_y)
{
    auto window_ptr = window_map.find(handle);
    if (window_ptr != window_map.end())
    {
        window_ptr->second->resize_window_internal(res_x, res_y);
    }
}

Surface::Surface(WindowParameters window_parameters) : window_name(window_parameters.application_name.c_str())
{
    std::lock_guard<std::mutex> lock(window_map_lock);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    if (window_parameters.b_is_fullscreen)
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Create window handle
    window_handle = glfwCreateWindow(window_parameters.size_x, window_parameters.size_y, window_parameters.application_name.c_str(), nullptr, nullptr);
    if (!window_handle)
        LOG_FATAL("failed to create glfw Surface");
    glfwSetFramebufferSizeCallback(window_handle, framebuffer_size_callback);

    // Create window surface
    create_window_surface();

    LOG_INFO("create new vulkan window");
    GfxContext::create<GfxContext>(surface);

    vulkan_common::set_msaa_sample_count(vulkan_utils::get_max_usable_sample_count());

    // Create window vulkan objects
    LOG_INFO("finished window creation");

    swapchain       = std::make_unique<Swapchain>(this);
    render_pass     = std::make_unique<RenderPass>(swapchain.get());
    back_buffer     = std::make_unique<Framebuffer>(render_pass.get(), swapchain.get());
    descriptor_pool = std::make_unique<DescriptorPool>(this);
    imgui_instance  = std::make_unique<ImGuiInstance>(this);

    swapchain->on_swapchain_recreate.add_object(this, &Surface::on_recreate_swapchain);

    window_map[window_handle] = this;

    LOG_VALIDATE("created Window '%s' ( %d x %d )", window_parameters.application_name.c_str(), window_parameters.size_x, window_parameters.size_y);
}

Surface::~Surface()
{
    std::lock_guard<std::mutex> lock(window_map_lock);
    window_map.erase(window_map.find(window_handle));

    // ensure all frames are submitted
    GfxContext::get()->wait_device();

    imgui_instance  = nullptr;
    descriptor_pool = nullptr;
    back_buffer     = nullptr;
    swapchain       = nullptr;
    render_pass     = nullptr;
    command_pool::destroy_pools();
    GfxContext::destroy();
    destroy_window_surface();

    glfwDestroyWindow(window_handle);
    LOG_VALIDATE("successfully destroyed window");
}

void Surface::resize_window_internal(const int res_x, const int res_y) const
{
    GfxContext::get()->wait_device();

    const uint32_t width  = res_x;
    const uint32_t height = res_y;

    swapchain->resize_swapchain({width, height});
}

SwapchainStatus Surface::prepare_frame() const
{
    if (swapchain->get_swapchain_extend().width == 0 || swapchain->get_swapchain_extend().height == 0)
        return {};

    auto render_context = swapchain->prepare_frame();
    if (!render_context.is_valid)
        return {};

    render_context.framebuffer = back_buffer->get(render_context.image_index);

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color        = {0.4f, 0.5f, 0.6f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo render_pass_info{
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass  = render_pass->get_render_pass(),
        .framebuffer = render_context.framebuffer,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = swapchain->get_swapchain_extend(),
            },
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues    = clear_values.data(),
    };
    VkViewport viewport{
        .x        = 0,
        .y        = static_cast<float>(swapchain->get_swapchain_extend().height), // Flip viewport vertically to avoid problem with imported models
        .width    = static_cast<float>(swapchain->get_swapchain_extend().width),
        .height   = -static_cast<float>(swapchain->get_swapchain_extend().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    VkRect2D scissor{
        .offset = VkOffset2D{0, 0},
        .extent = swapchain->get_swapchain_extend(),
    };

    vkCmdBeginRenderPass(render_context.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(render_context.command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(render_context.command_buffer, 0, 1, &scissor);

    return render_context;
}

void Surface::prepare_ui(SwapchainStatus& render_context) const
{
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Surface::render_data(SwapchainStatus& render_context) const
{
    ImGui::EndFrame();

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    BEGIN_NAMED_RECORD(RENDER_IMGUI_DATA);
    imgui_instance->ImGui_ImplVulkan_RenderDrawData(draw_data, render_context.command_buffer);
    END_NAMED_RECORD(RENDER_IMGUI_DATA);

    /************************************************************************/
    /* End imgui draw stuff                                                 */
    /************************************************************************/

    vkCmdEndRenderPass(render_context.command_buffer);
    VK_ENSURE(vkEndCommandBuffer(render_context.command_buffer), "Failed to register command buffer #d", render_context.image_index);

    swapchain->submit_frame(render_context);
}

bool Surface::begin_frame() const
{
    return !glfwWindowShouldClose(window_handle);
}

void Surface::create_window_surface()
{
    VK_ENSURE(glfwCreateWindowSurface(vulkan_common::instance, window_handle, nullptr, &surface) != VK_SUCCESS, "Failed to create Window surface");
    VK_CHECK(surface, "VkSurfaceKHR is null");
    LOG_INFO("Create Window surface");
}

void Surface::destroy_window_surface()
{
    LOG_INFO("Destroy window surface");
    vkDestroySurfaceKHR(vulkan_common::instance, surface, vulkan_common::allocation_callback);
}

void Surface::on_recreate_swapchain()
{
    back_buffer = std::make_unique<Framebuffer>(get_render_pass(), get_swapchain());
}

DescriptorPool* Surface::get_descriptor_pool() const

{
    return descriptor_pool.get();
}

GLFWwindow* Surface::get_handle() const

{
    return window_handle;
}

VkSurfaceKHR Surface::get_surface() const
{
    return surface;
}

RenderPass* Surface::get_render_pass() const
{
    return render_pass.get();
}

Swapchain* Surface::get_swapchain() const
{
    return swapchain.get();
}

VkExtent2D Surface::get_size() const
{
    return swapchain->get_swapchain_extend();
}