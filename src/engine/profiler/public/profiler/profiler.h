#pragma once
#include <chrono>

class Profiler final
{
  public:
    Profiler() : start_time(std::chrono::steady_clock::now()), duration_ms(0)
    {
    }

    ~Profiler();

    void restart();
    void end();
    double get_ms() const;
  private:
    std::chrono::steady_clock::time_point start_time;
    double                                duration_ms;
};
