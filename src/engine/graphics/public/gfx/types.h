#pragma once

#include <types/type_format.h>

namespace gfx
{
enum class EBufferType
{
    IMMUTABLE, // No allowed updates
    STATIC,    // Pretty never updated. Updating data would cause some freezes
    DYNAMIC,   // Data is stored internally, then automatically submitted. Can lead to a memory overhead depending on the buffer size.
    IMMEDIATE, // Data need to be submitted every frames
};

enum class EBufferUsage
{
    INDEX_DATA             = 0x00000001, // used as index buffer
    VERTEX_DATA            = 0x00000002, // used as vertex buffer
    GPU_MEMORY             = 0x00000003, // used as storage buffer
    UNIFORM_BUFFER         = 0x00000004, // used as uniform buffer
    INDIRECT_DRAW_ARGUMENT = 0x00000005, // used for indirect draw commands
    TRANSFER_MEMORY        = 0x00000006, // used for indirect draw commands
};

enum class EBufferAccess
{
    DEFAULT    = 0x00000000, // Choose best configuration
    GPU_ONLY   = 0x00000001, // Data will be cached on GPU
    CPU_TO_GPU = 0x00000002, // frequent transfer from CPU to GPU
    GPU_TO_CPU = 0x00000003, // frequent transfer from GPU to CPU
};

enum class EImageType
{
    Texture_1D,
    Texture_1D_Array,
    Texture_2D,
    Texture_2D_Array,
    Texture_3D,
    Cubemap,
};

enum class ETextureTransferCapabilities
{
    None            = 0x00000,
    CopySource      = 0x00001,
    CopyDestination = 0x00002,
};

enum class ETextureGPUWriteCapabilities
{
    None,
    Enabled,
};

enum class ETextureGPUReadCapabilities
{
    None,
    Sampling,
};
}