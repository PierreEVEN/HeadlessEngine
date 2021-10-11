#pragma once
#include <cpputils/logger.hpp>

template <typename Resource_T, typename GetMaxImageLambda_T> class SwapchainResource_Base
{
  public:
    SwapchainResource_Base() = default;
    SwapchainResource_Base(const Resource_T& default_value)
    {
        items.resize(GetMaxImageLambda_T(), default_value);
    }

    Resource_T& operator[](size_t index)
    {
        if (index >= items.size())
        {
            if (items.empty())
                items.resize(get_max_instance_count());
            else
                LOG_FATAL("cannot access swapchain resource at image index %lu (max is %u)", index, get_max_instance_count())
        }
        return items[index];
    }

    [[nodiscard]] static uint32_t get_max_instance_count()
    {
        return GetMaxImageLambda_T{}.get();
    }

  private:
    std::vector<Resource_T> items{};
};

struct SwapchainResourceImageCountHelper
{
    static uint32_t get();
};

struct SwapchainResourceInFlightImageHelper
{
    static uint32_t get();
};

template <typename Resource_T> using SwapchainImageResource = SwapchainResource_Base<Resource_T, SwapchainResourceImageCountHelper>;
template <typename Resource_T> using SwapchainInFlightResource = SwapchainResource_Base<Resource_T, SwapchainResourceInFlightImageHelper>;