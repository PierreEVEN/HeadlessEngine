#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gfx
{
using Handle = uint64_t;

using MatHandle = Handle;



class Device
{
public:
private:
    std::vector<MatHandle> materials;
};

Handle make_mat()
{
    return reinterpret_cast<MatHandle>(new std::string());
}

void delete_mat(MatHandle handle)
{
    delete reinterpret_cast<std::string*>(handle);
}




template <typename Resource_T> class TResourcehandle
{
public:

    void destroy()
    {
        should_destroy = true;
        if (!acquired)
            ; // destroy handle
    }

    void acquire()
    {
        acquired = true;
    }

    void release()
    {
        acquired = false;
        if (should_destroy)
            destroy();
    }

  private:
    bool should_destroy = false;
    bool acquired = false;
};
} // namespace gfx