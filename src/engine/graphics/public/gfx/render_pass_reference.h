#pragma once
#include <string>

#ifndef MAX_RENDER_PASS
#define MAX_RENDER_PASS 32
#endif

namespace gfx
{

class RenderPassID
{
    friend class RenderPass;

  public:
    static RenderPassID get(const std::string& pass_name);

    bool operator==(const RenderPassID& other) const
    {
        return other.id == id && other.is_valid && is_valid;
    }

    operator bool() const
    {
        return is_valid;
    }

    static bool is_valid_index(uint8_t index);

  private:
    static RenderPassID declare(const std::string& pass_name);

    RenderPassID(uint8_t new_id, bool valid) : is_valid(valid), id(new_id)
    {
    }

    bool    is_valid;
    uint8_t id;
};

template <typename Data_T> class RenderPassData
{
  public:
    class Iterator
    {
      public:
        Iterator(Data_T* in_data, uint8_t in_index) : data(in_data), index(in_index)
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
            } while (!RenderPassID::is_valid_index(index) && index != MAX_RENDER_PASS);
        }

        std::pair<uint8_t, Data_T>& operator*()
        {
            return {index, data[index]};
        }

        const std::pair<uint8_t, Data_T>& operator*() const
        {
            return {index, data[index]};
        }

      private:
        Data_T* data;
        uint8_t index;
    };

    Iterator begin()
    {
        return Iterator(data, 0);
    }

    Iterator end()
    {
        return Iterator(nullptr, MAX_RENDER_PASS);
    }

    RenderPassData()
    {
        clear();
    }

    Data_T& operator[](const RenderPassID& id)
    {
        return data[id];
    }

    const Data_T& operator[](const RenderPassID& id) const
    {
        return data[id];
    }

    void clear()
    {
        memset(data, 0, sizeof(Data_T) * MAX_RENDER_PASS);
    }

  private:
    Data_T data[MAX_RENDER_PASS];
};
} // namespace gfx
