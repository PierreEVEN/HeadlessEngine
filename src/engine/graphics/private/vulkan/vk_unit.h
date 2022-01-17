#pragma once

#include <cstdint>

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{
void    set_image_count(uint8_t image_count);
void    set_frame(uint8_t new_image_index);
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
    static SwapchainResource_Base make_static()
    {
        return SwapchainResource_Base(true);
    }
    static SwapchainResource_Base make_dynamic()
    {
        return SwapchainResource_Base(false);
    }

    SwapchainResource_Base() : SwapchainResource_Base(false)
    {
    }
    ~SwapchainResource_Base()
    {
        delete[] items;
        items = nullptr;
    }

    SwapchainResource_Base(const SwapchainResource_Base& other)
    {
        copy_from(other);
    }

    SwapchainResource_Base(SwapchainResource_Base&& other)
    {
        copy_from(other);
    }

    Resource_T& operator[](size_t index)
    {
        if (is_static())
            return items[0];
        return items[index];
    }

    const Resource_T& operator[](size_t index) const
    {
        if (is_static())
            return items[0];
        return items[index];
    }

    SwapchainResource_Base& operator=(const SwapchainResource_Base& other)
    {
        copy_from(other);
        return *this;
    }

    Resource_T& operator*()
    {
        if (is_static())
            return operator[](0);
        return operator[](GetMaxImageLambda_T::get_current_image());
    }

    const Resource_T& operator*() const
    {
        if (is_static())
            return operator[](0);
        return operator[](GetMaxImageLambda_T::get_current_image());
    }

    Resource_T* operator->()
    {
        if (is_static())
            return &operator[](0);
        return &operator[](GetMaxImageLambda_T::get_current_image());
    }

    const Resource_T* operator->() const
    {
        if (is_static())
            return &operator[](0);
        return &operator[](GetMaxImageLambda_T::get_current_image());
    }

    [[nodiscard]] uint8_t get_max_instance_count() const
    {
        if (is_static())
            return 1;
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

    [[nodiscard]] bool is_static() const
    {
        return resource_static;
    }

    Resource_T* data() const
    {
        return items;
    }

  private:
    void copy_from(const SwapchainResource_Base& other)
    {
        resource_static = other.resource_static;
        delete[] items;
        items = new Resource_T[get_max_instance_count()];
        memcpy(items, other.items, sizeof(Resource_T) * get_max_instance_count());
    }

    SwapchainResource_Base(bool make_resource_static) : resource_static(make_resource_static)
    {
        items = new Resource_T[get_max_instance_count()];
        for (uint8_t i = 0; i < get_max_instance_count(); ++i)
            new (items + i) Resource_T();
        std::memset(items, 0, sizeof(Resource_T) * get_max_instance_count());
    }

    bool        resource_static;
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