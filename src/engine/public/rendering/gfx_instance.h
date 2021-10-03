#pragma once

#define GLFW_INCLUDE_VULKAN
#include "renderer/renderer.h"
#include "renderer/render_pass_description.h"

#include <GLFW/glfw3.h>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vk_mem_alloc.h>

class DescriptorPool;
class SwapchainConfig;

struct WindowParameters
{
    int         size_x           = 800 ;
    int         size_y           = 600;
    bool        b_is_fullscreen  = false;
    std::string application_name = "Headless Engine - experimental build";
};

class QueueFamilyIndices
{
  public:
    QueueFamilyIndices() = default;
    std::optional<uint32_t> graphic_family;
    std::optional<uint32_t> transfert_family;
    std::optional<uint32_t> present_family;
    [[nodiscard]] bool      is_complete() const
    {
        return graphic_family.has_value() && present_family.has_value();
    }
};

class Renderer;
class Swapchain;

class GfxInterface
{
    friend class Graphics;

  public:
    GfxInterface()          = default;
    virtual ~GfxInterface() = default;

    void     submit_graphic_queue(const VkSubmitInfo& submit_infos, VkFence submit_fence);
    VkResult submit_present_queue(const VkPresentInfoKHR& present_infos);
    void     wait_device();

    virtual SwapchainConfig* define_swapchain_settings(VkSurfaceKHR surface);

    // test if the given extensions are supported by the given physical device
    static bool check_physical_device_extension_support(VkPhysicalDevice physical_device, const std::vector<const char*>& required_extensions);

    [[nodiscard]] VkPhysicalDevice   get_physical_device() const;
    [[nodiscard]] VkDevice           get_logical_device() const;
    [[nodiscard]] GLFWwindow*        get_glfw_handle() const;
    [[nodiscard]] VkSurfaceKHR       get_surface_khr() const;
    [[nodiscard]] QueueFamilyIndices get_queue_family_indices() const;
    [[nodiscard]] VmaAllocator       get_allocator() const;
    [[nodiscard]] SwapchainConfig*   get_swapchain_config() const;
    [[nodiscard]] Swapchain*         get_swapchain() const;
    [[nodiscard]] Renderer*          get_renderer() const;
    [[nodiscard]] DescriptorPool*    get_descriptor_pool() const;

    virtual SwapchainFrame begin_frame();
    virtual void           end_frame(const SwapchainFrame& swapchain_frame);

  protected:
    // Virtual constructor
    virtual void init(const WindowParameters& window_parameters);
    virtual void destroy();

    /**
     *  (1) CREATE WINDOW
     */

    // We only support GLFW at the moment
    virtual GLFWwindow* create_glfw_window(const WindowParameters& window_parameters);

    // Can be used to use you own surface KHR
    virtual VkSurfaceKHR create_surface_khr();

    /**
     *  (2) DEVICE INITIALIZATION
     */

    // Test if the given device is suitable for the application requirements
    virtual bool is_physical_device_suitable(VkSurfaceKHR surface, VkPhysicalDevice device);

    // Pick the most relevant physical device for our needs
    virtual VkPhysicalDevice select_physical_device(VkSurfaceKHR surface);

    // Find the queue families available on the given device
    virtual QueueFamilyIndices find_device_queue_families(VkSurfaceKHR surface, VkPhysicalDevice device);

    // Create the logical device
    virtual VkDevice create_logical_device(VkSurfaceKHR surface, VkPhysicalDevice device);

    /**
     *   (3) Create swapchain and renderer
     */

    virtual Swapchain* create_swapchain();

    virtual RendererConfiguration get_default_render_pass_configuration();

    virtual Renderer* create_renderer();

    virtual DescriptorPool* create_descriptor_pool();

  private:
    VmaAllocator                     vulkan_memory_allocator = VK_NULL_HANDLE;
    std::mutex                       queue_access_lock;
    std::shared_ptr<SwapchainConfig> swapchain_settings;

    /**
     * DEVICE DATA
     */

    VkPhysicalDevice   physical_device = VK_NULL_HANDLE;
    VkDevice           logical_device  = VK_NULL_HANDLE;
    QueueFamilyIndices queue_families  = {};
    VkQueue            graphic_queue   = VK_NULL_HANDLE;
    VkQueue            transfer_queue  = VK_NULL_HANDLE;
    VkQueue            present_queue   = VK_NULL_HANDLE;

    /**
     * INSTANCE DATA
     */
    //@TODO find best place for descriptor pool
    std::shared_ptr<DescriptorPool> descriptor_pool;

    // Renderer objects used by the main graphic context
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<Renderer>  renderer;

    // Window surface references
    GLFWwindow*  window_handle = nullptr;
    VkSurfaceKHR surface_khr   = VK_NULL_HANDLE;
};