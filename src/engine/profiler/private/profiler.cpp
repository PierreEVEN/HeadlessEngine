#include "profiler/profiler.h"

void Profiler::restart()
{
	start_time = std::chrono::steady_clock::now();
}

void Profiler::end()
{
    duration_ms = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_time).count()) / 1000000.0;
}

double Profiler::get_ms() const
{
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_time).count()) / 1000000.0;
}

Profiler::~Profiler()
{
    end();
}
