#pragma once

#include "debug/debug.h"

#include <cstdlib>

namespace cw::log
{
    enum LEVEL
    {
        LEVEL_DEBUG,
        LEVEL_INFO,
        LEVEL_WARNING,
        LEVEL_ERROR,
        LEVEL_FATAL,
        LEVEL_COUNT,
    };

    void Initialize();

    void Write(LEVEL level, const char* format, ...);
}

#define CW_DEBUG(message, ...) \
    cw::log::Write(cw::log::LEVEL_DEBUG, message __VA_OPT__(, __VA_ARGS__))

#define CW_INFO(message, ...) \
    cw::log::Write(cw::log::LEVEL_INFO, message __VA_OPT__(, __VA_ARGS__))

#define CW_WARNING(message, ...) \
    cw::log::Write(cw::log::LEVEL_WARNING, message __VA_OPT__(, __VA_ARGS__))

#define CW_ERROR(message, ...) \
    cw::log::Write(cw::log::LEVEL_ERROR, message __VA_OPT__(, __VA_ARGS__))

#define CW_FATAL(message, ...)                                                   \
    do                                                                           \
    {                                                                            \
        cw::log::Write(cw::log::LEVEL_FATAL, message __VA_OPT__(, __VA_ARGS__)); \
        CW_DEBUGBREAK();                                                          \
        abort();                                                                 \
    } while (0)
