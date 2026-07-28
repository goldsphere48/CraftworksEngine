#pragma once

#include "core/concepts.h"

#include <limits>

namespace cw::math
{
    template<CFloating T>
    inline constexpr T Epsilon = std::numeric_limits<T>::epsilon();

    template<CScalar T>
    constexpr T Abs(T value)
    {
        return value < T{0} ? -value : value;
    }

    template<CFloating T>
    constexpr bool NearlyZero(T value, T tolerance = Epsilon<T>)
    {
        return Abs(value) <= tolerance;
    }

    template<CInteger T>
    constexpr bool NearlyZero(T value)
    {
        return value == T{0};
    }

    template<CFloating T>
    constexpr bool NearlyEqual(T lhs, T rhs, T tolerance = Epsilon<T>)
    {
        if (lhs == rhs)
        {
            return true;
        }

        const T difference = Abs(lhs - rhs);

        if (difference <= tolerance)
        {
            return true;
        }

        const T largest = Abs(lhs) > Abs(rhs) ? Abs(lhs) : Abs(rhs);

        return difference <= tolerance * largest;
    }

    template<CInteger T>
    constexpr bool NearlyEqual(T lhs, T rhs)
    {
        return lhs == rhs;
    }
}
