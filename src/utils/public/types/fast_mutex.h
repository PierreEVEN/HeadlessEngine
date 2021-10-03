#pragma once
#include <mutex>

#include "cpputils/logger.hpp"

class FastMutex final
{
  public:
    FastMutex()                 = default;
    FastMutex(const FastMutex&) = delete;
    FastMutex& operator=(const FastMutex&) = delete;

    void lock()
    {
        for (;;)
        {
            if (!b_lock.exchange(true, std::memory_order_acquire))
            {
                break;
            }
            while (b_lock.load(std::memory_order_relaxed))
            {
            }
        }
    }

    void unlock()
    {
        b_lock.store(false, std::memory_order_release);
    }

  private:
    std::atomic_bool b_lock = {false};
};
