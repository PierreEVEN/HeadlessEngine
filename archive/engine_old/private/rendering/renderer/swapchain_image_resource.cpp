

#include "config.h"
#include "rendering/graphics.h"
#include "rendering/swapchain_config.h"
#include "rendering/renderer/swapchain_image_resource.h"

uint32_t SwapchainResourceImageCountHelper::get()
{
    if (!Graphics::get() || !Graphics::get()->get_swapchain_config())
        LOG_FATAL("Graphics have not been initialized yet");
    return Graphics::get()->get_swapchain_config()->get_image_count();
}

uint32_t SwapchainResourceInFlightImageHelper::get()
{
    return config::max_frame_in_flight;
}
