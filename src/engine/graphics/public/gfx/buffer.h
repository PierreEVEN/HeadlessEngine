#pragma once

#include "gfx/types.h"

#include <cstdint>
#include <memory>
#include <string>

namespace gfx
{
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

    // Get get element count
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

    // Resize the get
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