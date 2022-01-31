#pragma once
#include "device.h"

#include <string>
#include <cpputils/logger.hpp>

namespace gfx
{
class IGpuResource
{
    friend class Device;
  public:
    IGpuResource(std::string resource_name) : usage(0), should_destroy(false), name(std::move(resource_name))
    {
    }
    virtual ~IGpuResource() = default;

    void destroy()
    {
        std::lock_guard(safe_lock);
        should_destroy = true;
        if (usage == 0)
            Device::get().destroy_resource(this);
    }

    void release(uint8_t frame)
    {
        std::lock_guard(safe_lock);
        usage &= !frame;
        if (should_destroy)
            destroy();
    }

    IGpuResource(const IGpuResource&)  = delete;
    IGpuResource(const IGpuResource&&) = delete;
    IGpuResource& operator=(IGpuResource&) = delete;
    void          operator=(IGpuResource&&) = delete;

  protected:
    bool              should_destroy;
    const std::string name;
    uint8_t           usage;
    std::mutex        safe_lock;
};

template <typename Resource_T> class TGpuResource final : public IGpuResource
{
  public:
    template <typename... Ctor_T> TGpuResource(std::string resource_name, Ctor_T&&... args) : IGpuResource(std::move(resource_name)), resource(resource_name, std::forward<Ctor_T>(args)...)
    {
    }

    ~TGpuResource() override = default;

    Resource_T* use()
    {
        std::lock_guard(safe_lock);
        const auto frame_id = Device::get().get_frame_id();
        if (!(usage & frame_id))
        {
            usage |= frame_id;
            Device::get().acquire_resource(this);
        }
        return &resource;
    }

    Resource_T* try_modify()
    {
        std::lock_guard(safe_lock);
        if (usage != 0)
            return nullptr;
        return &resource;
    }
    
  private:
    Resource_T resource;
};

template <typename Resource_T, typename Handle_T> Resource_T* use_resource(const Handle_T handle)
{
    return dynamic_cast<TGpuResource<Resource_T>*>(handle)->use();
}

template <typename Resource_T, typename Handle_T> Resource_T* try_modify_resource(const Handle_T handle)
{
    return dynamic_cast<TGpuResource<Resource_T>*>(handle)->try_modify();
}
}
