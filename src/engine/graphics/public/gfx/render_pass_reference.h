#pragma once
#include <cpputils/logger.hpp>
#include <string>

#ifndef MAX_RENDER_PASS
#define MAX_RENDER_PASS 16
#endif

static_assert(MAX_RENDER_PASS <= 64, "max render pass count exceeded 64");

namespace gfx
{

class RenderPassID
{
    friend class RenderPass;

    template <typename Data_T> friend class RenderPassData;

  public:
    static RenderPassID get(const std::string& pass_name);
    static bool         exists(const std::string& pass_name);

    bool operator==(const RenderPassID& other) const;
    bool operator!=(const RenderPassID& other) const;
         operator bool() const;

    [[nodiscard]] std::string name() const;
    [[nodiscard]] uint8_t     get_internal_num() const;

    static std::vector<RenderPassID> get_all();

  private:
    static RenderPassID declare(const std::string& pass_name);
    explicit RenderPassID(uint8_t new_id) : internal_id(new_id)
    {
    }
    uint8_t internal_id;
};

template <typename Data_T> class RenderPassData
{
  public:
    class Iterator
    {
      public:
        Iterator(Data_T* in_data, uint8_t in_index, uint64_t& in_valid_map) : data(in_data), index(in_index), valid_map(in_valid_map)
        {
        }

        bool operator==(const Iterator& other) const
        {
            return index == other.index;
        }

        void operator++()
        {
            do
            {
                index++;
            } while (!(valid_map & 1LL << index) && index != MAX_RENDER_PASS);
        }

        Data_T& operator*()
        {
            return data[index];
        }

        const Data_T& operator*() const
        {
            return data[index];
        }

        Data_T* operator->()
        {
            return &data[index];
        }

        const Data_T* operator->() const
        {
            return &data[index];
        }

        [[nodiscard]] RenderPassID id() const
        {
            return RenderPassID(index);
        }


      private:
        Data_T*   data;
        uint8_t   index;
        uint64_t& valid_map;
    };

    Iterator begin()
    {
        uint8_t start = 0;
        while (!contains(RenderPassID(start)) && start != MAX_RENDER_PASS)
            start++;
        return Iterator(data, start, valid_pass_bitmap);
    }

    Iterator end()
    {
        return Iterator(data + MAX_RENDER_PASS, MAX_RENDER_PASS, valid_pass_bitmap);
    }

    [[nodiscard]] bool contains(const RenderPassID& id) const
    {
        return valid_pass_bitmap & 1LL << id.get_internal_num();
    }

    RenderPassData& operator=(const RenderPassData& other) = delete;

    Data_T& operator[](const RenderPassID& id)
    {
        if (!contains(id))
            LOG_FATAL("render pass %s is not valid in the current context. Please gfx_update it first", id.name().c_str());
        return data[id.get_internal_num()];
    }

    const Data_T& operator[](const RenderPassID& id) const
    {
        if (!contains(id))
            LOG_FATAL("render pass %s is not valid in the current context. Please gfx_update it first", id.name().c_str());
        return data[id.get_internal_num()];
    }

    template <typename... Args_T> Data_T& init(const RenderPassID& id, Args_T&&... args)
    {
        if (!id)
            LOG_FATAL("render pass %s is not valid.", id.name().c_str());

        if (contains(id))
            destroy(id);

        new (data + id.get_internal_num()) Data_T(std::forward<Args_T>(args)...);
        valid_pass_bitmap |= 1LL << id.get_internal_num();
        return data[id.get_internal_num()];
    }

    void destroy(const RenderPassID& id)
    {
        data[id.get_internal_num()].~Data_T();
        valid_pass_bitmap &= ~(1LL << id.get_internal_num());
    }

    void clear()
    {
        for (uint8_t i = 0; i < MAX_RENDER_PASS; ++i)
            if (contains(RenderPassID(i)))
                destroy(RenderPassID(i));
        valid_pass_bitmap = 0;
        memset(data, 0, sizeof(Data_T) * MAX_RENDER_PASS);
    }

    RenderPassData()
    {
        clear();
    }

  private:
    Data_T   data[MAX_RENDER_PASS];
    uint64_t valid_pass_bitmap = 0;
};
} // namespace gfx
