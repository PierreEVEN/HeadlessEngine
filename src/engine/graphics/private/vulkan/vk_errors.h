#pragma once

#include <cpputils/logger.hpp>
#include <types/magic_enum.h>

#define VK_CHECK(condition, text, ...)                                                            \
    if ((condition) != VK_SUCCESS)                                                                \
    {                                                                                             \
        LOG_FATAL("%s : %s", magic_enum::enum_name(static_cast<VkResult>(condition)).data(), stringutils::format(text, __VA_ARGS__).c_str()); \
    }