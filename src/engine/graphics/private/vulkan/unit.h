#pragma once

#include <cstdint>

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{
void    set_image_count(uint8_t image_count);
void    next_frame();
uint8_t get_image_index();
uint8_t get_image_count();

template <typename Resource_T> class SwapchainResourceIterator
{
  public:
    SwapchainResourceIterator(Resource_T* elem) : element(elem)
    {
    }

    Resource_T& operator++()
    {
        return *++element;
    }

    Resource_T& operator*()
    {
        return *element;
    }

    [[nodiscard]] bool operator==(const SwapchainResourceIterator<Resource_T>& other) const
    {
        return this->element == other.element;
    }

    [[nodiscard]] bool operator!=(const SwapchainResourceIterator<Resource_T>& other) const
    {
        return !(*this == other);
    }

  private:
    Resource_T* element;
};

template <typename Resource_T, typename GetMaxImageLambda_T> class SwapchainResource_Base final
{
  public:
    SwapchainResource_Base()
    {
        items = new Resource_T[get_max_instance_count()];
        LOG_WARNING("create %x", items);
    };
    ~SwapchainResource_Base()
    {
        LOG_WARNING("delete %x", items);
        delete[] items;
        items = nullptr;
        LOG_VALIDATE("done");
    }

    SwapchainResource_Base(const SwapchainResource_Base& other)
    {
        items = new Resource_T[get_max_instance_count()];
        memcpy(items, other.items, sizeof(Resource_T) * get_max_instance_count());
    }

    SwapchainResource_Base(SwapchainResource_Base&& other)
    {
        items = new Resource_T[get_max_instance_count()];
        memcpy(items, other.items, sizeof(Resource_T) * get_max_instance_count());
    }

    Resource_T& operator[](size_t index)
    {
        return items[index];
    }

    const Resource_T& operator[](size_t index) const
    {
        return items[index];
    }

    Resource_T& operator*()
    {
        return operator[](GetMaxImageLambda_T::get_current_image());
    }

    SwapchainResource_Base& operator=(const SwapchainResource_Base& other)
    {
        delete[] items;
        items = new Resource_T[get_max_instance_count()];
        memcpy(items, other.items, sizeof(Resource_T) * get_max_instance_count());
        return *this;
    }

    const Resource_T& operator*() const
    {
        return operator[](GetMaxImageLambda_T::get_current_image());
    }

    Resource_T* operator->()
    {
        return &operator[](GetMaxImageLambda_T::get_current_image());
    }

    [[nodiscard]] static uint8_t get_max_instance_count()
    {
        return GetMaxImageLambda_T::get();
    }

    SwapchainResourceIterator<Resource_T> begin()
    {
        return SwapchainResourceIterator(items);
    }

    SwapchainResourceIterator<Resource_T> end()
    {
        return SwapchainResourceIterator(items + get_max_instance_count());
    }

  private:
    Resource_T* items = nullptr;
};

struct SwapchainResourceImageCountHelper
{
    static uint8_t get()
    {
        return get_image_count();
    }

    static uint8_t get_current_image()
    {
        return get_image_index();
    }
};

template <typename Resource_T> using SwapchainImageResource = SwapchainResource_Base<Resource_T, SwapchainResourceImageCountHelper>;
} // namespace gfx::vulkan