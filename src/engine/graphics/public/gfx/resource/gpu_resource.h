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

        bool        should_destroy  = false;
        uint32_t    reference_count = 1;
        std::string name;

        IResourceReference(const IResourceReference& other) = delete;
        IResourceReference(IResourceReference&& other)      = delete;
        IResourceReference& operator=(const IResourceReference& other) = delete;
        IResourceReference& operator=(IResourceReference&& other) = delete;

      protected:
        IResourceReference()          = default;
        virtual ~IResourceReference() = default;
    };

  public:
    IGpuHandle() : resource_ptr(nullptr)
    {
    }
    IGpuHandle(IResourceReference* in_resource) : resource_ptr(in_resource)
    {
    }

    virtual ~IGpuHandle()
    {
        decrement_ref_count();
    }

    IGpuHandle(const IGpuHandle& other)
    {
        decrement_ref_count();
        resource_ptr = other.resource_ptr;
        increment_ref_count();
    }
    IGpuHandle(const IGpuHandle&& other) noexcept
    {
        decrement_ref_count();
        resource_ptr = other.resource_ptr;
        increment_ref_count();
    }
    IGpuHandle& operator=(const IGpuHandle& rhs)
    {
        if (this == &rhs)
            return *this;

        decrement_ref_count();
        resource_ptr = rhs.resource_ptr;
        increment_ref_count();
        return *this;
    }
    IGpuHandle& operator=(IGpuHandle&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        decrement_ref_count();
        resource_ptr = rhs.resource_ptr;
        increment_ref_count();
        return *this;
    }

    void destroy();

  protected:
    IResourceReference* resource_ptr = nullptr;

  private:
    void increment_ref_count()
    {
        if (resource_ptr)
            ++resource_ptr->reference_count;
    }

    void decrement_ref_count()
    {
        if (resource_ptr)
        {
            --resource_ptr->reference_count;
            if (resource_ptr->reference_count == 0)
                destroy();
        }
    }
};

template <typename Resource_T> class TGpuHandle final : public IGpuHandle
{
    struct TResourceReference : IResourceReference
    {
        template <typename... Args_T> TResourceReference(std::string in_name, Args_T&&... args) : resource(in_name, std::forward<Args_T>(args)...)
        {
        }

        Resource_T resource;
    };

  public:
    template <typename... Args_T> TGpuHandle(const std::string& in_name, Args_T&&... args) : IGpuHandle(new TResourceReference(in_name, std::forward<Args_T>(args)...))
    {
        resource_ptr->name = in_name;
    }

    TGpuHandle() : IGpuHandle()
    {
    }

    operator bool() const
    {
        return resource_ptr != nullptr;
    }

    Resource_T* operator->()
    {
        if (!dynamic_cast<TResourceReference*>(resource_ptr))
            LOG_FATAL("trying to access null resource");
        return &(dynamic_cast<TResourceReference*>(resource_ptr)->resource);
    }

    const Resource_T* operator->() const
    {
        if (!resource_ptr)
            LOG_FATAL("trying to access null resource");
        return &dynamic_cast<TResourceReference*>(resource_ptr)->resource;
    }

    Resource_T& operator*()
    {
        if (!resource_ptr)
            LOG_FATAL("trying to access null resource");
        return dynamic_cast<TResourceReference*>(resource_ptr)->resource;
    }

    const Resource_T& operator*() const
    {
        if (!resource_ptr)
            LOG_FATAL("trying to access null resource");
        return dynamic_cast<TResourceReference*>(resource_ptr)->resource;
    }
};
} // namespace gfx
