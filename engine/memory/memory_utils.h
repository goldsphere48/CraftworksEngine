#pragma once

#include "utils/macros.h"
#include <memory.h>

namespace cw::memory
{
    CW_FORCEINLINE void MemZero(void* ptr, size_t size)
    {
        memset(ptr, 0, size);
    }
    
    template<typename T>
    void MemZeroStruct<T>(T* ptr)
    {
        memset(ptr, 0, sizeof(T));
    }
}
