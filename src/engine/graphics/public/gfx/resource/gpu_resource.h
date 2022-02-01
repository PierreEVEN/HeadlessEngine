#pragma once
#include "gfx/resource/device.h"

#include <cpputils/logger.hpp>
#include <string>

namespace gfx
{
class IGpuResource
{
  public:
    IGpuResource(std::string resource_name) : should_destroy(false), name(std::move(resource_name)), usage(0)
    {
    }
    virtual ~IGpuResource()
    {
    }

    void destroy()
    {
        should_destroy = true;
        if (usage == 0)
        {
            Device::get().destroy_resource(this);
        }
    }

    void release(uint8_t frame)
    {
        safe_lock.lock();
        usage &= ~(1 << frame);
        if (should_destroy && usage == 0)
        {
            safe_lock.unlock();
            Device::get().destroy_resource(this);
            return;
        }
        safe_lock.unlock();
    }

    IGpuResource(const IGpuResource&)  = delete;
    IGpuResource(const IGpuResource&&) = delete;
    IGpuResource& operator=(IGpuResource&) = delete;
    void          operator=(IGpuResource&&) = delete;

    template <typename Resource_T> Resource_T* use();

    template <typename Resource_T> Resource_T* edit();

    [[nodiscard]] const std::string& get_name() const
    {
        return name;
    }

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
        Device::get().register_resource(this);
    }

    ~TGpuResource() override = default;

    Resource_T* try_use()
    {
        std::lock_guard lock(safe_lock);
        const auto      frame_id = Device::get().get_current_frame();
        if ((usage & 1 << frame_id) == 0)
        {
            usage |= 1 << frame_id;
            Device::get().acquire_resource(this);
        }
        return &resource;
    }

    Resource_T* try_modify()
    {
        std::lock_guard lock(safe_lock);
        if (usage != 0)
            return nullptr;
        return &resource;
    }

  private:
    Resource_T resource;
};

template <typename Resource_T> Resource_T* IGpuResource::use()
{
    return should_destroy ? nullptr : dynamic_cast<TGpuResource<Resource_T>*>(this)->try_use();
}

template <typename Resource_T> Resource_T* IGpuResource::edit()
{
    return should_destroy ? nullptr : dynamic_cast<TGpuResource<Resource_T>*>(this)->try_modify();
}

} // namespace gfx
