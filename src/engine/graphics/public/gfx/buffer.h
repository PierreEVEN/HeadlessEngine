#pragma once
#include <cstdint>
#include <memory>
#include <string>

namespace gfx
{

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

class Buffer
{
  public:
    static std::shared_ptr<Buffer> create(const std::string& buffer_name, uint32_t element_count, uint32_t stride, EBufferUsage usage, EBufferAccess buffer_access = EBufferAccess::DEFAULT);
    static std::shared_ptr<Buffer> create(const std::string& buffer_name, uint32_t buffer_size, EBufferUsage usage, EBufferAccess buffer_access = EBufferAccess::DEFAULT);

    virtual ~Buffer();

    [[nodiscard]] size_t get_size() const
    {
        return stride * element_count;
    }
    virtual void set_data(void* data, size_t data_length, size_t offset = 0) = 0;
    
    template <typename T> void set_data(T& data)
    {
        set_data(&data, sizeof(T));
    }

    [[nodiscard]] uint32_t count() const
    {
        return element_count;
    }

    [[nodiscard]] uint32_t get_stride() const
    {
        return stride;
    }
  protected:
    Buffer(const std::string& buffer_name, uint32_t buffer_stride, uint32_t elements, EBufferUsage buffer_usage, EBufferAccess in_buffer_access = EBufferAccess::DEFAULT);

    virtual void* get_ptr()     = 0;
    virtual void  submit_data() = 0;

    uint32_t      stride;
    uint32_t      element_count;
    std::string   buffer_name;
    EBufferAccess buffer_access;
    EBufferUsage  usage;
};
} // namespace gfx