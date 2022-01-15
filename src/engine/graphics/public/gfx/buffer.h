#pragma once
#include <cstdint>
#include <memory>
#include <string>

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

class Buffer
{
  public:
    static std::shared_ptr<Buffer> create(const std::string& buffer_name, uint32_t element_count, uint32_t stride, EBufferUsage usage, EBufferAccess buffer_access = EBufferAccess::DEFAULT,
                                          EBufferType buffer_type = EBufferType::IMMUTABLE);
    static std::shared_ptr<Buffer> create(const std::string& buffer_name, uint32_t buffer_size, EBufferUsage usage, EBufferAccess buffer_access = EBufferAccess::DEFAULT, EBufferType buffer_type = EBufferType::IMMUTABLE);

    virtual ~Buffer();

    // Get data total size
    [[nodiscard]] size_t size() const
    {
        return element_stride * element_count;
    }

    // Get buffer element count
    [[nodiscard]] uint32_t count() const
    {
        return element_count;
    }

    // Get per element size
    [[nodiscard]] uint32_t stride() const
    {
        return element_stride;
    }


    // Set data into a callback
    template <typename Lambda = void> void set_data(Lambda callback)
    {
        callback(acquire_data_ptr());
        submit_data();
    }

    // Resize the buffer
    virtual void resize(uint32_t element_count) = 0;

  protected:
    Buffer(const std::string& buffer_name, uint32_t buffer_stride, uint32_t elements, EBufferUsage buffer_usage, EBufferAccess in_buffer_access, EBufferType buffer_type);

    virtual void* acquire_data_ptr() = 0;
    virtual void  submit_data()      = 0;

    uint32_t      element_stride;
    uint32_t      element_count;
    std::string   buffer_name;
    EBufferAccess buffer_access;
    EBufferUsage  usage;
    EBufferType   type;
};
} // namespace gfx