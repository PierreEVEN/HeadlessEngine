#pragma once
#include <string>
#include <cpputils/logger.hpp>

namespace gfx
{
class IGpuResource
{
  public:
    IGpuResource(std::string resource_name) : used(false), should_destroy(false), name(std::move(resource_name))
    {
    }
    virtual ~IGpuResource() = default;
    void destroy();

    IGpuResource(const IGpuResource&)  = delete;
    IGpuResource(const IGpuResource&&) = delete;
    IGpuResource& operator=(IGpuResource&) = delete;
    void          operator=(IGpuResource&&) = delete;

  protected:
    bool              used;
    bool              should_destroy;
    const std::string name;
};

template <typename Resource_T> class TGpuResource final : public IGpuResource
{
  public:
    template <typename... Ctor_T> TGpuResource(std::string resource_name, Ctor_T&&... args) : IGpuResource(std::move(resource_name)), resource(resource_name, std::forward<Ctor_T>(args)...)
    {
    }

    ~TGpuResource() override = default;

    Resource_T& use()
    {
        used = true;
        return resource;
    }

    Resource_T& modify()
    {
        if (used)
        {
            LOG_ERROR("resource already in use");
            return nullptr;
        }
        return resource;
    }

    void release()
    {
        used = false;
        if (should_destroy)
            destroy();
    }
    
  private:
    Resource_T resource;
};

template <typename Resource_T, typename Handle_T> TGpuResource<Resource_T>* handle_cast(const Handle_T handle)
{
    return dynamic_cast<TGpuResource<Resource_T>*>(handle);
}
}
