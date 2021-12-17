#pragma once

#include <vulkan/vulkan.hpp>

namespace gfx::vulkan
{
enum class EQueueFamilyType
{
    GRAPHIC_QUEUE = 0,
    COMPUTE_QUEUE = 1,
    TRANSFER_QUEUE = 2,
};

inline std::vector<uint32_t> get_supported_queues()
{
    return {};
}


struct QueueInfo
{
    bool is_valid;
    VkQueue queue;
};

}