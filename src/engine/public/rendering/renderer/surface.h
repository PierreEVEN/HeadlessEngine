#pragma once

#include "swapchain.h"
#include <GLFW/glfw3.h>
#include <string>

class NodeBase;
class WindowBase;
class DescriptorPool;
class ImGuiInstance;
class Framebuffer;
class Swapchain;
class RenderPass;

struct WindowParameters
{
    int         size_x           = 800;
    int         size_y           = 600;
    bool        b_is_fullscreen  = false;
    std::string application_name = "Headless Engine - experimental build";
};

class Surface
{
    friend WindowBase;
    friend void framebuffer_size_callback(GLFWwindow* handle, int res_x, int res_y);

  public:
    Surface(WindowParameters window_parameters = WindowParameters{});
    virtual ~Surface();

    bool                          begin_frame() const;
    [[nodiscard]] SwapchainStatus prepare_frame() const;
    void                          prepare_ui(SwapchainStatus& render_context) const;
    void                          render_data(SwapchainStatus& render_context) const;

    [[nodiscard]] DescriptorPool* get_descriptor_pool() const;
    [[nodiscard]] GLFWwindow*     get_handle() const;
    [[nodiscard]] VkSurfaceKHR    get_surface() const;
    [[nodiscard]] RenderPass*     get_render_pass() const;
    [[nodiscard]] Swapchain*      get_swapchain() const;
    [[nodiscard]] VkExtent2D      get_size() const;

  private:
    void resize_window_internal(int res_x, int res_y) const;

    const char*                     window_name;
    GLFWwindow*                     window_handle   = nullptr;
    VkSurfaceKHR                    surface         = VK_NULL_HANDLE;
    std::unique_ptr<DescriptorPool> descriptor_pool = nullptr;
    std::unique_ptr<ImGuiInstance>  imgui_instance  = nullptr;
    std::unique_ptr<Swapchain>      swapchain       = nullptr;
    std::unique_ptr<RenderPass>     render_pass     = nullptr;
    std::unique_ptr<Framebuffer>    back_buffer     = nullptr;

    void create_window_surface();
    void destroy_window_surface();
    void on_recreate_swapchain();
};
