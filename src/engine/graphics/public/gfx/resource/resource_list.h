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

// Texture view
struct CI_TextureView
{
};
using TextureViewHandle = ResourceHandle;

// Shader
struct CI_Shader
{
};
using ShaderHandle = ResourceHandle;

// Render pass
struct CI_RenderPass
{
};
using RenderPassHandle = ResourceHandle;

// Pipeline layout
struct CI_PipelineLayout
{
};
using PipelineLayoutHandle = ResourceHandle;

// Sampler
struct CI_Sampler
{
};
using SamplerHandle = ResourceHandle;

// Pipeline
struct CI_Pipeline
{
};
using PipelineHandle = ResourceHandle;

// Descriptor set layout
struct CI_DescriptorSetLayout
{
};
using DescriptorSetLayoutHandle = ResourceHandle;

// Framebuffer
struct CI_Framebuffer
{
};
using FramebufferHandle = ResourceHandle;

// Command pool
struct CI_CommandPool
{
};
using CommandPoolHandle = ResourceHandle;

// Descriptor pool
struct CI_DescriptorPool
{
};
using DescriptorPoolHandle = ResourceHandle;

// Texture surface
struct CI_Surface
{
};
using SurfaceHandle = ResourceHandle;

// Texture swapchain
struct CI_Swapchain
{
};
using SwapchainHandle = ResourceHandle;
} // namespace gfx