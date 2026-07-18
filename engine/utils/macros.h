#pragma once

#if defined(_MSC_VER)
    #define CW_FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define CW_FORCEINLINE __attribute__((always_inline)) inline
#else
    #define CW_FORCEINLINE inline
#endif

#define CW_UNUSED(x) ((void)(x))
