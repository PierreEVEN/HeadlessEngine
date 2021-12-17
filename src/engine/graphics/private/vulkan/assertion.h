#pragma once

#include <cpputils/logger.hpp>

#define VK_CHECK(condition, ...)                                \
    if ((condition) != VK_SUCCESS)                             \
    {                                                          \
        LOG_FATAL("VK_ERROR %d : %s", condition, __VA_ARGS__); \
    }

namespace gfx
{

}