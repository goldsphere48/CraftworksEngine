#pragma once

#include "core/types.h"

namespace cw::utils
{
    constexpr uint64 HashString(const char* str)
    {
        uint64 hash = 14695981039346656037ull;
        while (*str != '\0')
        {
            hash = (hash ^ static_cast<uint8>(*str++)) * 1099511628211ull;
        }

        return hash;
    }
}
