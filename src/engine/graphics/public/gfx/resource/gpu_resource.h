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
            Device::get().destroy_resource(this);
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

class IGpuHandle
{
  protected:
    friend class TestDevice;
    struct IResourceReference
    {
        friend class TestDevice;

        bool     should_destroy  = false;
        uint32_t reference_count = 1;

      protected:
        IResourceReference() = default;
        virtual ~IResourceReference();
    };

  public:
    IGpuHandle() : resource(nullptr)
    {
    }

    virtual ~IGpuHandle()
    {
        decrement_ref_count();
    }

    IGpuHandle(const IGpuHandle& other)
    {
        decrement_ref_count();

        resource = other.resource;
        if (resource)
            ++resource->reference_count;
    }
    IGpuHandle(const IGpuHandle&& other) noexcept
    {
        decrement_ref_count();

        resource = other.resource;
        if (resource)
            ++resource->reference_count;
    }
    IGpuHandle& operator=(const IGpuHandle& rhs)
    {
        if (this == &rhs)
            return *this;

        decrement_ref_count();

        resource = rhs.resource;
        if (resource)
            ++resource->reference_count;
        return *this;
    }
    IGpuHandle& operator=(IGpuHandle&& rhs) noexcept
    {
        decrement_ref_count();

        resource = rhs.resource;
        if (resource)
            ++resource->reference_count;
        return *this;
    }

    void destroy();

  protected:
    IResourceReference* resource;

  private:
    void decrement_ref_count()
    {
        if (resource)
        {
            --resource->reference_count;
            if (resource->reference_count == 0)
                destroy();
        }
    }
};

template <typename Resource_T> class TGpuHandle final : public IGpuHandle
{
    struct TResourceReference : IResourceReference
    {
        template <typename... Args_T> TResourceReference(std::string in_name, Args_T&&... args) : resource(Resource_T(in_name, std::forward<Args_T>(args)...))
        {
        }

        Resource_T resource;
    };

  public:
    template <typename... Args_T> TGpuHandle(const std::string& in_name, Args_T&&... args) : resource(new TResourceReference(in_name, std::forward<Args_T>(args)...))
    {
    }

    TGpuHandle() = default;

    Resource_T* operator->()
    {
        return &dynamic_cast<TResourceReference*>(resource)->resource;
    }

    Resource_T& operator*()
    {
        return dynamic_cast<TResourceReference*>(resource)->resource;
    }
};

class TestDevice
{
  public:
    static void delete_resource(IGpuHandle::IResourceReference* resource)
    {
        deletion_queues[current_frame].emplace_back(resource);
    }

    void begin_frame(uint8_t frame_index)
    {
        for (const auto& resource : deletion_queues[current_frame])
            delete resource;
        deletion_queues[current_frame].clear();

        current_frame = frame_index;
    }

  private:
    static std::vector<std::vector<IGpuHandle::IResourceReference*>> deletion_queues;
    static uint8_t                                                   current_frame;
};

class toto
{
  public:
    void test_to()
    {
    }
};

void use_examples()
{
    TGpuHandle<toto> test_resource("toto");

    test_resource->test_to();

    test_resource = TGpuHandle<toto>();
}

} // namespace gfx
