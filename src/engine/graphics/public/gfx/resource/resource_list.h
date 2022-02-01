#pragma once

#define GPU_NULL_HANDLE nullptr;
#include "gfx/types.h"

namespace gfx
{
class IGpuResource;

using ResourceHandle = IGpuResource*;

// Buffer
struct CI_Buffer
{
    uint32_t      stride = 1;
    uint32_t      count  = 1;
    EBufferUsage  usage  = EBufferUsage::GPU_MEMORY;
    EBufferAccess access = EBufferAccess::DEFAULT;
    EBufferType   type   = EBufferType::IMMUTABLE;
};
using BufferHandle = ResourceHandle;

// Buffer View
struct CI_BufferView
{
};
using BufferViewHandle = ResourceHandle;

// Command Buffer
struct CI_CommandBuffer
{
};
using CommandBufferHandle = ResourceHandle;

// Queues
struct CI_Queue
{
};
using QueueHandle = ResourceHandle;

// Semaphore
struct CI_Semaphore
{
};
using SemaphoreHandle = ResourceHandle;

// Fence
struct CI_Fence
{
};
using FenceHandle = ResourceHandle;

// Texture
struct CI_Texture
{
};
using TextureHandle = ResourceHandle;

// Texture View
struct CI_TextureView
{
};
using TextureViewHandle = ResourceHandle;

// Texture View
struct CI_Shader
{
};
using ShaderHandle = ResourceHandle;

// Texture View
struct CI_RenderPass
{
};
using RenderPassHandle = ResourceHandle;

// Texture View
struct CI_PipelineLayout
{
};
using PipelineLayoutHandle = ResourceHandle;

// Texture View
struct CI_Sampler
{
};
using Sampler_Handle = ResourceHandle;

// Texture View
struct CI_Pipeline
{
};
using PipelineHandle = ResourceHandle;

// Texture View
struct CI_DescriptorSetLayout
{
};
using DescriptorSetLayoutHandle = ResourceHandle;

/*
else if (typeid(VkDescriptorPool) == typeid(Object_T))
else if (typeid(VkDescriptorPool) == typeid(Object_T))
else if (typeid(VkFramebuffer) == typeid(Object_T))
else if (typeid(VkCommandPool) == typeid(Object_T))
else if (typeid(VkSurfaceKHR) == typeid(Object_T))
else if (typeid(VkSwapchainKHR) == typeid(Object_T))
*/

} // namespace gfx