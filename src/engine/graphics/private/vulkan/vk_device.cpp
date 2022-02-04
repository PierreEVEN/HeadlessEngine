
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "gfx/resource/gpu_resource.h"
#include "vulkan/vk_device.h"

#include "types/magic_enum.h"
#include "vk_buffer.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_helper.h"
#include "vk_instance.h"
#include "vk_types.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_config.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_physical_device.h"

#include <cpputils/logger.hpp>
#include <set>

namespace gfx::vulkan
{
static VkImageUsageFlags vk_usage(const CI_Texture& texture_parameters)
{
    VkImageUsageFlags usage_flags = 0;
    if (static_cast<int>(texture_parameters.transfer_capabilities) & static_cast<int>(ETextureTransferCapabilities::CopySource))
        usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (static_cast<int>(texture_parameters.transfer_capabilities) & static_cast<int>(ETextureTransferCapabilities::CopyDestination))
        usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (texture_parameters.gpu_write_capabilities == ETextureGPUWriteCapabilities::Enabled)
        usage_flags |= is_depth_format(texture_parameters.image_format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (texture_parameters.gpu_read_capabilities == ETextureGPUReadCapabilities::Sampling)
        usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

    return usage_flags;
}

FenceResource_VK::FenceResource_VK(const std::string& name, const CI_Fence& create_infos)
{
    const VkFenceCreateInfo fence_infos{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    vkCreateFence(get_device(), &fence_infos, get_allocator(), &fence);
}

FenceResource_VK::~FenceResource_VK()
{
    vkDestroyFence(get_device(), fence, get_allocator());
}

SemaphoreResource_VK::SemaphoreResource_VK(const std::string& name, const CI_Semaphore& create_infos)
{
    const VkSemaphoreCreateInfo semaphore_infos{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    vkCreateSemaphore(get_device(), &semaphore_infos, get_allocator(), &semaphore);
}

SemaphoreResource_VK::~SemaphoreResource_VK()
{
    vkDestroySemaphore(get_device(), semaphore, get_allocator());
}

void Device_VK::init()
{
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
    float                                queue_priorities   = 1.0f;

    get_physical_device<PhysicalDevice_VK>()->update_queues();
    const auto queues = get_physical_device<PhysicalDevice_VK>()->get_queues();

    std::set<int> queue_indices = {};
    for (const auto queue : queues)
        queue_indices.insert(queue->queue_index);

    for (const auto queue : queue_indices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queue;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queue_priorities;
        queue_create_infos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures device_features
    {
#if ENABLE_VALIDATION_LAYER
        .robustBufferAccess = VK_TRUE,
#endif
        .geometryShader       = VK_TRUE,
        .sampleRateShading    = VK_TRUE, // Sample Shading
            .fillModeNonSolid = VK_TRUE, // Wireframe
            .wideLines = VK_TRUE, .samplerAnisotropy = VK_TRUE,
    };

    VkDeviceCreateInfo create_infos = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = NULL,
        .queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos       = queue_create_infos.data(),
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = static_cast<uint32_t>(config::device_extensions.size()),
        .ppEnabledExtensionNames = config::device_extensions.begin(),
        .pEnabledFeatures        = &device_features,
    };

    VK_CHECK(vkCreateDevice(GET_VK_PHYSICAL_DEVICE(), &create_infos, get_allocator(), &device), "failed to create device");
    debug_set_object_name("primary device", device);

    std::unordered_map<uint32_t, SwapchainImageResource<VkFence>> queue_fence_map;
    for (const auto& queue : queues)
    {
        queue_fence_map[queue->queue_index] = {};
        vkGetDeviceQueue(device, queue->queue_index, 0, &queue->queues);
    }

    VkFenceCreateInfo fence_infos{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (auto& [index, fence] : queue_fence_map)
    {
        uint8_t i = 0;
        for (auto& item : fence)
        {
            VK_CHECK(vkCreateFence(get_device(), &fence_infos, get_allocator(), &item), "Failed to create fence");
            debug_set_object_name(stringutils::format("fence queue submit : queue=%d #%d", index, i++), item);
        }
    }

    for (const auto& queue : queues)
        queue->queue_submit_fence = queue_fence_map[queue->queue_index];

    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = GET_VK_PHYSICAL_DEVICE(),
        .device         = get_device(),
        .instance       = get_instance(),
    };
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vulkan_memory_allocator), "failed to create vma allocator");
}

Device_VK::~Device_VK()
{
    free_allocations();

    command_pool::destroy_pools();

    const auto queues = get_physical_device<PhysicalDevice_VK>()->get_queues();

    std::unordered_map<uint32_t, SwapchainImageResource<VkFence>> queue_fence_map;
    for (const auto& queue : queues)
        queue_fence_map[queue->queue_index] = queue->queue_submit_fence;

    for (auto& [index, fence] : queue_fence_map)
        for (auto& item : fence)
            vkDestroyFence(get_device(), item, get_allocator());

    VmaStats stats;
    vulkan_memory_allocator->CalculateStats(&stats);
    if (stats.total.allocationCount > 0)
        LOG_FATAL("%d allocation were not destroyed", stats.total.allocationCount);
    vmaDestroyAllocator(vulkan_memory_allocator);

    vkDestroyDevice(device, get_allocator());
    device = VK_NULL_HANDLE;
}

BufferHandle Device_VK::create_buffer(const std::string& name, const CI_Buffer& create_infos)
{
    return new TGpuResource<BufferResource_VK>(name, create_infos);
}

CommandBufferHandle Device_VK::create_command_buffer(const std::string& name, const CI_CommandBuffer& create_infos)
{
    return new TGpuResource<CommandBufferResource_VK>(name, create_infos);
}

TextureHandle Device_VK::create_texture(const std::string& name, const CI_Texture& create_infos)
{
    return new TGpuResource<ImageResource_VK>(name, create_infos);
}

void Device_VK::wait_device()
{
    if (device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(device);
}

BufferResource_VK::BufferResource_VK(const std::string& name, const CI_Buffer& create_infos)
{
    VkBufferUsageFlags vk_usage  = 0;
    VmaMemoryUsage     vma_usage = VMA_MEMORY_USAGE_UNKNOWN;

    switch (create_infos.usage)
    {
    case EBufferUsage::INDEX_DATA:
        vk_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case EBufferUsage::VERTEX_DATA:
        vk_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case EBufferUsage::GPU_MEMORY:
        vk_usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    case EBufferUsage::UNIFORM_BUFFER:
        vk_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case EBufferUsage::INDIRECT_DRAW_ARGUMENT:
        vk_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        break;
    case EBufferUsage::TRANSFER_MEMORY:
        vk_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        break;
    }

    if (create_infos.type != EBufferType::IMMUTABLE)
    {
        vk_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        vk_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    switch (create_infos.access)
    {
    default:
    case EBufferAccess::DEFAULT:
        vma_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
    case EBufferAccess::GPU_ONLY:
        vma_usage = VMA_MEMORY_USAGE_GPU_ONLY;
        break;
    case EBufferAccess::CPU_TO_GPU:
        vma_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
    case EBufferAccess::GPU_TO_CPU:
        vma_usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        break;
    }

    const VkBufferCreateInfo buffer_create_info = {
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = NULL,
        .size                  = create_infos.count * create_infos.stride,
        .usage                 = vk_usage,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
    };

    const VmaAllocationCreateInfo allocInfo = {
        .flags          = NULL,
        .usage          = vma_usage,
        .requiredFlags  = NULL,
        .preferredFlags = NULL,
        .memoryTypeBits = 0,
        .pool           = VK_NULL_HANDLE,
        .pUserData      = nullptr,
    };
    VK_CHECK(vmaCreateBuffer(get_vma_allocator(), &buffer_create_info, &allocInfo, &buffer, &memory, nullptr), "failed to create get shreked");
    debug_set_object_name(name, buffer);
}

BufferResource_VK::~BufferResource_VK()
{
    vmaDestroyBuffer(get_vma_allocator(), buffer, memory);
}

void* BufferResource_VK::map_buffer()
{
    void* dst_ptr;
    VK_CHECK(vmaMapMemory(get_vma_allocator(), memory, &dst_ptr), "failed to map memory");
    return dst_ptr;
}

void BufferResource_VK::unmap_buffer()
{
    vmaUnmapMemory(get_vma_allocator(), memory);
}

ImageResource_VK::ImageResource_VK(const std::string& name, const CI_Texture& create_infos)
{
    if (create_infos.existing_texture_handle)
    {
        image        = static_cast<VkImage>(create_infos.existing_texture_handle);
        memory       = VK_NULL_HANDLE;
        image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    else
    {
        VkImageCreateInfo image_infos{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .format        = vk_texture_format_to_engine(create_infos.image_format),
            .mipLevels     = create_infos.mip_level,
            .samples       = VK_SAMPLE_COUNT_1_BIT,
            .tiling        = VK_IMAGE_TILING_OPTIMAL,
            .usage         = vk_usage(create_infos),
            .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        switch (create_infos.image_type)
        {
        case EImageType::Texture_1D:
            image_infos.imageType = VK_IMAGE_TYPE_1D;
            image_infos.extent    = {
                .width  = create_infos.width,
                .height = 1,
                .depth  = 1,
            };
            image_infos.arrayLayers = 1;
            break;
        case EImageType::Texture_1D_Array:
            image_infos.imageType = VK_IMAGE_TYPE_1D;
            image_infos.extent    = {
                .width  = create_infos.width,
                .height = 1,
                .depth  = 1,
            };
            image_infos.arrayLayers = create_infos.depth;
            break;
        case EImageType::Texture_2D:
            image_infos.imageType = VK_IMAGE_TYPE_2D;
            image_infos.extent    = {
                .width  = create_infos.width,
                .height = create_infos.height,
                .depth  = 1,
            };
            image_infos.arrayLayers = 1;
            break;
        case EImageType::Texture_2D_Array:
            image_infos.imageType = VK_IMAGE_TYPE_2D;
            image_infos.extent    = {
                .width  = create_infos.width,
                .height = create_infos.height,
                .depth  = 1,
            };
            image_infos.arrayLayers = create_infos.depth;
            break;
        case EImageType::Texture_3D:
            image_infos.imageType = VK_IMAGE_TYPE_3D;
            image_infos.extent    = {
                .width  = create_infos.width,
                .height = create_infos.height,
                .depth  = create_infos.depth,
            };
            image_infos.arrayLayers = 1;
            break;
        case EImageType::Cubemap:
            image_infos.imageType = VK_IMAGE_TYPE_2D;
            image_infos.extent    = {
                .width  = create_infos.width,
                .height = create_infos.height,
                .depth  = 1,
            };
            image_infos.arrayLayers = 6;
            break;
        }
        const VmaAllocationCreateInfo vma_allocation{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };
        vmaCreateImage(get_vma_allocator(), &image_infos, &vma_allocation, &image, &memory, nullptr);
    }
}

ImageResource_VK::~ImageResource_VK()
{
    if (memory != VK_NULL_HANDLE)
        vmaDestroyImage(get_vma_allocator(), image, memory);
}

ImageViewResource_VK::ImageViewResource_VK(const std::string& name, const CI_TextureView& create_infos)
{
    VkImageViewCreateInfo image_view_infos{
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .format     = vk_texture_format_to_engine(image_parameters.format),
        .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
        .subresourceRange =
            {
                .aspectMask     = is_depth_format(image_parameters.format) ? static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT) : static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT),
                .baseMipLevel   = 0,
                .levelCount     = image_parameters.mip_level.value(),
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
    };

    switch (image_parameters.image_type)
    {
    case EImageType::Texture_1D:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_1D;
        break;
    case EImageType::Texture_1D_Array:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        break;
    case EImageType::Texture_2D:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case EImageType::Texture_2D_Array:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        break;
    case EImageType::Texture_3D:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case EImageType::Cubemap:
        image_view_infos.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        break;
    }
    image_view_infos.image = images[i];
    vkCreateImageView(get_device(), &image_view_infos, get_allocator(), &views[i]);

    for (auto& infos : image_descriptor_info)
    {
        infos.sampler     = VK_NULL_HANDLE;
        infos.imageView   = views[i];
        infos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}

ImageViewResource_VK::~ImageViewResource_VK()
{
}
} // namespace gfx::vulkan