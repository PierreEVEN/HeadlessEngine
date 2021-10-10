
#include "rendering/gfx_instance.h"

#include "engine_interface.h"
#include "rendering/graphics.h"
#include "rendering/renderer/renderer.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/command_pool.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/descriptor_pool.h"
#include "rendering/vulkan/utils.h"
#include <config.h>
#include <cpputils/logger.hpp>
#include <set>

void framebuffer_size_callback(GLFWwindow* handle, const int res_x, const int res_y)
{
    Graphics::get()->get_swapchain()->resize_swapchain(VkExtent2D{static_cast<uint32_t>(res_x), static_cast<uint32_t>(res_y)});
}

void GfxInterface::init(const WindowParameters& window_parameters)
{
    LOG_INFO("[ GFX] : create window");
    // Create window
    window_handle = create_glfw_window(window_parameters);
    if (!window_handle)
        LOG_FATAL("failed to create glfw Surface");

    glfwSetFramebufferSizeCallback(window_handle, framebuffer_size_callback);

    LOG_INFO("[ GFX] : create surface khr");
    // Create surface interface
    surface_khr = create_surface_khr();

    LOG_INFO("[ GFX] : pick physical device");
    // Select physical device
    physical_device = select_physical_device(surface_khr);
    VK_CHECK(physical_device, "Cannot find any suitable GPU");
    VkPhysicalDeviceProperties selected_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &selected_device_properties);
    LOG_INFO("[ GFX] Picking physical device %d (%s)", selected_device_properties.deviceID, selected_device_properties.deviceName);

    LOG_INFO("[ GFX] : pick swapchain settings");
    // retrieve swapchain settings
    swapchain_settings = std::unique_ptr<SwapchainConfig>(define_swapchain_settings(surface_khr));

    LOG_INFO("[ GFX] : create logical device");
    // Create logical device and retrieve device queues
    logical_device = create_logical_device(surface_khr, physical_device);
    if (queue_families.transfert_family.has_value())
        vkGetDeviceQueue(logical_device, queue_families.transfert_family.value(), 0, &transfer_queue);
    if (queue_families.graphic_family.has_value())
        vkGetDeviceQueue(logical_device, queue_families.graphic_family.value(), 0, &graphic_queue);
    if (queue_families.present_family.has_value())
        vkGetDeviceQueue(logical_device, queue_families.present_family.value(), 0, &present_queue);

    LOG_INFO("[ GFX] : create vulkan memory allocator");
    // Create VMA allocator
    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = physical_device,
        .device         = logical_device,
        .instance       = vulkan_common::instance,
    };
    vmaCreateAllocator(&allocatorInfo, &vulkan_memory_allocator);

    vulkan_common::set_msaa_sample_count(vulkan_utils::get_max_usable_sample_count());

    LOG_INFO("[ GFX] : create descriptor pool");
    descriptor_pool = std::shared_ptr<DescriptorPool>(create_descriptor_pool());

    LOG_INFO("[ GFX] : create swapchain");
    // Create swapchain
    swapchain = std::shared_ptr<Swapchain>(create_swapchain());
    swapchain->on_swapchain_recreate.add_lambda([]() {
        const auto swapchain_extend = Graphics::get()->get_swapchain()->get_swapchain_extend();
        Graphics::get()->get_renderer()->init_or_resize(swapchain_extend);
        LOG_INFO("recreate swapchain : (%d x %d)", swapchain_extend.width, swapchain_extend.height);
    });

    LOG_INFO("[ GFX] : create renderer");
    renderer = std::shared_ptr<Renderer>(create_renderer());
    renderer->set_render_pass_description(get_default_render_pass_configuration());
    renderer->init_or_resize(swapchain->get_swapchain_extend());

    LOG_VALIDATE("[ GFX] : successfully initialized graphics");
}

void GfxInterface::destroy()
{
    LOG_INFO("[ GFX] : destroy renderer");
    renderer = nullptr;

    LOG_INFO("[ GFX] : destroy swapchain");
    swapchain = nullptr;

    LOG_INFO("[ GFX] : destroy descriptor pool");
    descriptor_pool = nullptr;

    LOG_INFO("[ GFX] : destroy vulkan memory allocator");
    vmaDestroyAllocator(vulkan_memory_allocator);
    vulkan_memory_allocator = VK_NULL_HANDLE;

    LOG_INFO("[ GFX] : destroy command pools");
    command_pool::destroy_pools();

    LOG_INFO("[ GFX] : destroy logical device");
    vkDestroyDevice(logical_device, vulkan_common::allocation_callback);
    logical_device = VK_NULL_HANDLE;

    swapchain_settings = nullptr;

    physical_device = VK_NULL_HANDLE;

    LOG_INFO("[ GFX] : destroy surface khr");
    vkDestroySurfaceKHR(vulkan_common::instance, surface_khr, vulkan_common::allocation_callback);
    surface_khr = VK_NULL_HANDLE;

    LOG_INFO("[ GFX] : destroy window");
    glfwDestroyWindow(window_handle);
    window_handle = nullptr;

    LOG_VALIDATE("[ GFX] : successfully destroyed graphics");
}

GLFWwindow* GfxInterface::create_glfw_window(const WindowParameters& window_parameters)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    if (window_parameters.b_is_fullscreen)
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Create window handle
    return glfwCreateWindow(window_parameters.size_x, window_parameters.size_y, window_parameters.application_name.c_str(), nullptr, nullptr);
}

VkSurfaceKHR GfxInterface::create_surface_khr()
{
    VkSurfaceKHR surface;
    VK_ENSURE(glfwCreateWindowSurface(vulkan_common::instance, window_handle, vulkan_common::allocation_callback, &surface) != VK_SUCCESS, "Failed to create Window surface");
    return surface;
}

void GfxInterface::submit_graphic_queue(const VkSubmitInfo& submit_infos, VkFence submit_fence)
{
    std::lock_guard<std::mutex> lock(queue_access_lock);
    VK_ENSURE(vkQueueSubmit(graphic_queue, 1, &submit_infos, submit_fence), "Failed to submit graphic queue");
}

VkResult GfxInterface::submit_present_queue(const VkPresentInfoKHR& present_infos)
{
    std::lock_guard<std::mutex> lock(queue_access_lock);
    return vkQueuePresentKHR(present_queue, &present_infos);
}

void GfxInterface::wait_device()
{
    std::lock_guard queue_lock(queue_access_lock);
    vkDeviceWaitIdle(logical_device);
}

SwapchainConfig* GfxInterface::define_swapchain_settings(VkSurfaceKHR surface)
{
    return new SwapchainConfig(surface);
}

VkPhysicalDevice GfxInterface::select_physical_device(VkSurfaceKHR surface)
{
    // Get devices
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vulkan_common::instance, &device_count, nullptr);
    if (device_count == 0)
        LOG_FATAL("No graphical device found.");
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(vulkan_common::instance, &device_count, devices.data());

    // Enumerate devices
    std::string PhysLog = "Found " + std::to_string(device_count) + " graphical devices : \n";
    for (const VkPhysicalDevice& device : devices)
    {
        VkPhysicalDeviceProperties pProperties;
        vkGetPhysicalDeviceProperties(device, &pProperties);
        PhysLog += "\t-" + std::string(pProperties.deviceName) + " (driver version : " + std::to_string(pProperties.driverVersion) + ")\n";
    }
    LOG_INFO("[ Core] %s", PhysLog.c_str());

    // Pick desired device
    for (const auto& device : devices)
    {
        if (is_physical_device_suitable(surface, device))
        {
            VkPhysicalDeviceProperties pProperties;
            vkGetPhysicalDeviceProperties(device, &pProperties);
            // if (pProperties.deviceName[0] == 'G') continue;

            return device;
        }
    }
    return VK_NULL_HANDLE;
}

bool GfxInterface::check_physical_device_extension_support(VkPhysicalDevice physical_device, const std::vector<const char*>& required_extensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

    for (const auto& ext : required_extensions)
    {
        bool bContains = false;
        for (const auto& extension : availableExtensions)
        {
            if (!std::strcmp(ext, extension.extensionName))
                bContains = true;
        }
        if (!bContains)
            return false;
    }

    return true;
}

VkPhysicalDevice GfxInterface::get_physical_device() const
{
    VK_CHECK(physical_device, "physical device is null");
    return physical_device;
}

VkDevice GfxInterface::get_logical_device() const
{
    VK_CHECK(physical_device, "logical device is null");
    return logical_device;
}

GLFWwindow* GfxInterface::get_glfw_handle() const
{
    if (!window_handle)
        LOG_FATAL("window have not been created yet");
    return window_handle;
}

VkSurfaceKHR GfxInterface::get_surface_khr() const
{
    VK_CHECK(surface_khr, "surface khr has not been created yet");
    return surface_khr;
}

QueueFamilyIndices GfxInterface::get_queue_family_indices() const
{
    return queue_families;
}

VmaAllocator GfxInterface::get_allocator() const
{
    VK_CHECK(physical_device, "memory allocator is null");
    return vulkan_memory_allocator;
}

SwapchainConfig* GfxInterface::get_swapchain_config() const
{
    if (!swapchain_settings)
        LOG_FATAL("swapchain setting object is null");
    return swapchain_settings.get();
}

Swapchain* GfxInterface::get_swapchain() const
{
    if (!swapchain)
        LOG_FATAL("swapchain is null");
    return swapchain.get();
}

Renderer* GfxInterface::get_renderer() const
{
    if (!renderer)
        LOG_FATAL("renderer is null");
    return renderer.get();
}

DescriptorPool* GfxInterface::get_descriptor_pool() const
{
    if (!descriptor_pool)
        LOG_FATAL("descriptor pool is null");
    return descriptor_pool.get();
}

SwapchainFrame GfxInterface::begin_frame()
{
    // acquire next frame
    const auto render_context = swapchain->acquire_frame();
    if (!render_context.is_valid)
        return {};

    return render_context;
}

void GfxInterface::end_frame(const SwapchainFrame& swapchain_frame)
{
    if (!swapchain_frame.is_valid)
        return;

    // submit frame
    swapchain->submit_frame(swapchain_frame);
}

bool GfxInterface::is_physical_device_suitable(VkSurfaceKHR surface, VkPhysicalDevice device)
{

    QueueFamilyIndices indices = find_device_queue_families(surface, device);

    const bool bAreExtensionSupported = check_physical_device_extension_support(device, config::required_device_extensions);

    bool swapChainAdequate = false;
    if (bAreExtensionSupported)
    {
        vulkan_utils::SwapchainSupportDetails swapChainSupport = vulkan_utils::get_swapchain_support_details(surface, device);
        swapChainAdequate                                      = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.is_complete() && bAreExtensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices GfxInterface::find_device_queue_families(VkSurfaceKHR surface, VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphic_family = i;
        }
        if (!indices.transfert_family.has_value() && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            indices.transfert_family = i;
        }

        if (queueFamily.queueFlags == VK_QUEUE_TRANSFER_BIT)
        {
            indices.transfert_family = i;
        }
        VkBool32 presentSupport = false;
        VK_ENSURE(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport), "failed to get physical device present support");
        if (presentSupport)
        {
            indices.present_family = i;
        }

        if (indices.is_complete())
        {
            break;
        }

        i++;
    }
    if (!indices.is_complete())
    {
        LOG_FATAL("queue family indices are not complete");
    }
    return indices;
}

VkDevice GfxInterface::create_logical_device(VkSurfaceKHR surface, VkPhysicalDevice device)
{
    queue_families = find_device_queue_families(surface, device);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set                             unique_queue_families = {queue_families.graphic_family.value(), queue_families.present_family.value()};
    float                                queue_priorities      = 1.0f;
    for (uint32_t queueFamily : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queue_priorities;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{
        .geometryShader    = VK_TRUE,
        .sampleRateShading = VK_TRUE, // Sample Shading
        .fillModeNonSolid  = VK_TRUE, // Wireframe
        .wideLines         = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };

    VkDeviceCreateInfo createInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos       = queueCreateInfos.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(config::required_device_extensions.size()),
        .ppEnabledExtensionNames = config::required_device_extensions.data(),
        .pEnabledFeatures        = &deviceFeatures,
    };

    if (config::use_validation_layers)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(config::required_validation_layers.size());
        createInfo.ppEnabledLayerNames = config::required_validation_layers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VkDevice log_device = VK_NULL_HANDLE;
    VK_ENSURE(vkCreateDevice(physical_device, &createInfo, vulkan_common::allocation_callback, &log_device), "Failed to create logical device");

    return log_device;
}

Swapchain* GfxInterface::create_swapchain()
{
    return new Swapchain(this);
}

RendererConfiguration GfxInterface::get_default_render_pass_configuration()
{
    return IEngineInterface::get()->get_default_render_pass_configuration();
}

Renderer* GfxInterface::create_renderer()
{
    return new Renderer(get_swapchain());
}

DescriptorPool* GfxInterface::create_descriptor_pool()
{
    return new DescriptorPool();
}
