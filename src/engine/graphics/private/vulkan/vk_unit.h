#pragma once

#include "gfx/resource/device.h"

#include <cstdint>

#include <cpputils/logger.hpp>

namespace gfx::vulkan
{

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

template <typename Resource_T, typename GetMaxImageLambda_T> class TSwapchainResource_Base final
{
  public:
    static TSwapchainResource_Base make_static()
    {
        return TSwapchainResource_Base(true);
    }

    static TSwapchainResource_Base make_dynamic()
    {
        return TSwapchainResource_Base(false);
    }

    TSwapchainResource_Base() : TSwapchainResource_Base(false)
    {
    }

    ~TSwapchainResource_Base()
    {
        delete[] items;
        items = nullptr;
    }

    TSwapchainResource_Base(const TSwapchainResource_Base& other)
    {
        copy_from(other);
    }

    TSwapchainResource_Base(TSwapchainResource_Base&& other)
    {
        move_from(std::move(other));
    }

    TSwapchainResource_Base& operator=(const TSwapchainResource_Base& other)
    {
        if (&other == this)
            return *this;

        copy_from(other);
        return *this;
    }
    TSwapchainResource_Base& operator=(TSwapchainResource_Base&& other)
    {
        if (&other == this)
            return *this;

        move_from(std::move(other));
        return *this;
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

    [[nodiscard]] Resource_T* data() const
    {
        return items;
    }

  private:
    void move_from(TSwapchainResource_Base&& other)
    {
        prepare_alloc(other);
        for (uint8_t i = 0; i < get_max_instance_count(); ++i)
            items[i] = std::move(other.items[i]);
    }

    void copy_from(const TSwapchainResource_Base& other)
    {
        prepare_alloc(other);
        for (uint8_t i = 0; i < get_max_instance_count(); ++i)
            items[i] = other.items[i];
    }

    void prepare_alloc(const TSwapchainResource_Base& other)
    {
        resource_static = other.resource_static;
        if (items == nullptr)
            items = new Resource_T[get_max_instance_count()];
    }

    TSwapchainResource_Base(bool make_resource_static) : resource_static(make_resource_static), items(new Resource_T[get_max_instance_count()])
    {
        for (uint8_t i = 0; i < get_max_instance_count(); ++i)
            new (items + i) Resource_T();
    }

    bool        resource_static;
    Resource_T* items = nullptr;
};

struct SwapchainResourceImageCountHelper
{
    static uint8_t get()
    {
        return Device::get().get_frame_count();
    }

    static uint8_t get_current_image()
    {
        return Device::get().get_current_frame();
    }
};

template <typename Resource_T> using SwapchainImageResource = TSwapchainResource_Base<Resource_T, SwapchainResourceImageCountHelper>;
} // namespace gfx::vulkan
