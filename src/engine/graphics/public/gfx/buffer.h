#pragma once
#include <cstdint>
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
    Buffer(const std::string& buffer_name, uint32_t size, EBufferUsage usage, EBufferAccess buffer_access = EBufferAccess::DEFAULT);
    virtual ~Buffer();

    [[nodiscard]] size_t get_size() const
    {
        return buffer_size;
    }
    void set_data(void* data, size_t data_length, size_t offset = 0);

    template<typename Lambda_T> void set_data(Lambda_T lambda)
    {
        lambda(get_ptr());
        submit_data();
    }

    template <typename T> void set_data(T& data)
    {
        set_data(&data, sizeof(T));
    }

  private:
    void* get_ptr();
    void  submit_data();

    void   create_buffer_internal(uint32_t buffer_size, EBufferUsage buffer_usage, EBufferAccess buffer_access);
    size_t buffer_size;

    std::string buffer_name;

    using Buffer_t = uint64_t;
    using Memory_t = uint64_t;

    EBufferAccess buffer_access;
    Buffer_t      buffer_handle;
    Memory_t      buffer_memory;
};
} // namespace gfx