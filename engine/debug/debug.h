#pragma once

#include "logger/log.h"

#include <cstdlib>

#if defined(CW_PLATFORM_WINDOWS)
    #define CW_DEBUGBREAK() __debugbreak()
#elif defined(__clang__)
    #define CW_DEBUGBREAK() __builtin_debugtrap()
#else
    #include <csignal>
    #define CW_DEBUGBREAK() raise(SIGTRAP)
#endif

#if defined(CW_BUILD_DEBUG)
    #define CW_ASSERT(condition, ...)                                            \
        do                                                                       \
        {                                                                        \
            if (!(condition))                                                    \
            {                                                                    \
                cw::log::Write(cw::log::LEVEL_FATAL,                             \
                    "Assertion failed: (%s), %s:%d",                             \
                    #condition, __FILE__, __LINE__);                             \
                __VA_OPT__(cw::log::Write(cw::log::LEVEL_FATAL, __VA_ARGS__);)   \
                CW_DEBUGBREAK();                                                  \
                abort();                                                         \
            }                                                                    \
        } while (0)
#else
    #define CW_ASSERT(condition, ...) ((void)sizeof(condition))
#endif
