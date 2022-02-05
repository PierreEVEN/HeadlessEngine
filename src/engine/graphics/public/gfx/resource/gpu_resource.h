#pragma once
#include <cpputils/logger.hpp>
#include <string>

namespace gfx
{
class IGpuHandle
{
  protected:
    friend class Device;
    struct IResourceReference
    {
        friend class Device;

        bool     should_destroy  = false;
        uint32_t reference_count = 1;

      protected:
        IResourceReference()          = default;
        virtual ~IResourceReference() = default;
    };

  public:
    IGpuHandle() = default;
    IGpuHandle(IResourceReference* in_resource) : resource(in_resource)
    {
    }

    virtual ~IGpuHandle()
    {
        decrement_ref_count();
    }

    IGpuHandle(const IGpuHandle& other)
    {
        // Free last resource
        decrement_ref_count();

        // store new referenced one
        resource = other.resource;

        // Increment new resource reference counter
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
    IResourceReference* resource = nullptr;

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
    template <typename... Args_T> TGpuHandle(const std::string& in_name, Args_T&&... args) : IGpuHandle(new TResourceReference(in_name, std::forward<Args_T>(args)...))
    {
    }

    TGpuHandle() = default;

    operator bool() const
    {
        return resource != nullptr;
    }

    Resource_T* operator->()
    {
        return &dynamic_cast<TResourceReference*>(resource)->resource;
    }

    const Resource_T* operator->() const
    {
        return &dynamic_cast<TResourceReference*>(resource)->resource;
    }

    Resource_T& operator*()
    {
        return dynamic_cast<TResourceReference*>(resource)->resource;
    }

    const Resource_T& operator*() const
    {
        return dynamic_cast<TResourceReference*>(resource)->resource;
    }
};
} // namespace gfx
