#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(CW_PLATFORM_WINDOWS)
    #include <io.h>
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace cw::log
{
    static bool g_colored = true;

    constexpr const char* CW_ANSI_RESET = "\x1b[0m";

    const char* g_logLevelColor[LEVEL_COUNT] =
        {"\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[41;37m"};

    const char* g_logLevelTag[LEVEL_COUNT] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "FATAL",
    };

    static bool SupportsAnsiColors()
    {
#ifdef CW_PLATFORM_WINDOWS
        if (!_isatty(_fileno(stdout)))
            return false;
#else
        if (!isatty(fileno(stdout)))
            return false;
#endif

        if (getenv("NO_COLOR") != nullptr)
            return false;

        const char* term = getenv("TERM");
        if (term != nullptr && strcmp(term, "dumb") == 0)
            return false;

        return true;
    }

    void Initialize()
    {
        g_colored = SupportsAnsiColors();
    }

    void Write(LEVEL level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);

        char message[1024];
        vsnprintf(message, sizeof(message), format, args);

        char formatted[2048];
        if (g_colored)
        {
            snprintf(formatted, sizeof(formatted), "%s[%s] %s%s\n", g_logLevelColor[level], g_logLevelTag[level], message, CW_ANSI_RESET);
        }
        else
        {
            snprintf(formatted, sizeof(formatted), "[%s] %s\n", g_logLevelTag[level], message);
        }

        FILE* channel = level >= LEVEL_ERROR ? stderr : stdout;
        fprintf(channel, "%s", formatted);
        fflush(channel);
        
        va_end(args);
    }
}
