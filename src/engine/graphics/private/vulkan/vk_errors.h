#pragma once

#include <cpputils/logger.hpp>

#define VK_CHECK(condition, text, ...)                                                            \
    if ((condition) != VK_SUCCESS)                                                                \
    {                                                                                             \
        LOG_FATAL("VK_ERROR %d : %s", condition, stringutils::format(text, __VA_ARGS__).c_str()); \
    }