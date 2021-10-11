#include "cpputils/logger.hpp"
#include "engine_interface.h"
#include "main_game_interface.h"

/*
#define pthread_create(...)
#define pthread_exit(...)
#define pthread_join(...)

const int n = 10;
const int p = 3;
int       results[n];
int       threads[n];

void* thread_func(void* val)
{
    int t = (int)*(int64_t*)val;
    int ut    = 0;
    for (int i = 1; i <= p; ++i)
    {
        ut += ((t - 1) * p + i);
    }
    results[t - 1] = ut;
    pthread_exit(NULL);
}

void calc() {

    for (int i = 1; i <= n; ++i)
        pthread_create(threads + i - 1, NULL, &thread_func, (void*)(int64_t)i);

    for (int i = 1; i <= n; ++i)
        pthread_join(threads + i - 1, NULL);

    int result = 0;

    for (int i = 0; i < n; ++i)
        result += results[i];
}
*/

int main(int argc, char* argv[])
{
    IEngineInterface::enable_logs(Logger::LOG_LEVEL_TRACE | Logger::LOG_LEVEL_DEBUG | Logger::LOG_LEVEL_INFO);
    IEngineInterface::run<MainGameInterface>();
}
