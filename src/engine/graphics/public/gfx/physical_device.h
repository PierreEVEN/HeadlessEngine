#pragma once
#include <string>
#include <vector>

namespace gfx
{
enum class EPhysicalDeviceType
{
    UNKNOWN,
    INTEGRATED_GPU,
    DEDICATED_GPU,
    VIRTUAL_GPU,
    CPU,
};

enum class EPhysicalDeviceFeature
{
    GPU //@TODO
};

class PhysicalDevice
{
  public:
    [[nodiscard]] std::string get_device_name() const
    {
        return device_name;
    }

    [[nodiscard]] uint32_t get_api_version() const
    {
        return api_version;
    }

    [[nodiscard]] uint32_t get_driver_version() const
    {
        return driver_version;
    }

    [[nodiscard]] uint32_t get_vendor_id() const
    {
        return vendor_id;
    }

    [[nodiscard]] uint32_t get_device_id() const
    {
        return device_id;
    }

    [[nodiscard]] EPhysicalDeviceType get_device_type() const
    {
        return device_type;
    }

  protected:
    std::string         device_name    = "";
    uint32_t            api_version    = 0;
    uint32_t            driver_version = 0;
    uint32_t            vendor_id      = 0;
    uint32_t            device_id      = 0;
    EPhysicalDeviceType   device_type    = EPhysicalDeviceType::UNKNOWN;
};

void fetch_physical_devices();

PhysicalDevice* find_best_physical_device(const std::vector<PhysicalDevice*>& candidates);

std::vector<PhysicalDevice*> get_physical_devices();

void select_physical_device(PhysicalDevice* device);

PhysicalDevice*                        get_physical_device_internal();
template <typename Device_T> Device_T* get_physical_device()
{
    return static_cast<Device_T*>(get_physical_device_internal());
}

} // namespace gfx
