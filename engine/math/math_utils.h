#pragma once

#include "core/concepts.h"
#include "core/types.h"

#include <limits>

namespace cw::math
{
    template<CFloating T>
    inline constexpr T Epsilon = std::numeric_limits<T>::epsilon();

    inline constexpr float Pi = 3.14159265358979323846f;

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

    template<CScalar T, usize N>
    requires(N > 0) struct Vector;

    template<CScalar T, usize R, usize C>
    requires(R > 0 && C > 0) struct Matrix;

    template<CFloating T, usize N>
    constexpr bool NearlyEqual(
        const Vector<T, N>& lhs, const Vector<T, N>& rhs, T tolerance = Epsilon<T>
    )
    {
        for (usize i = 0; i < N; ++i)
        {
            if (!NearlyEqual(lhs.Data[i], rhs.Data[i], tolerance))
            {
                return false;
            }
        }

        return true;
    }

    template<CInteger T, usize N>
    constexpr bool NearlyEqual(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        for (usize i = 0; i < N; ++i)
        {
            if (lhs.Data[i] != rhs.Data[i])
            {
                return false;
            }
        }

        return true;
    }

    template<CFloating T, usize N>
    constexpr bool NearlyZero(const Vector<T, N>& vector, T tolerance = Epsilon<T>)
    {
        for (usize i = 0; i < N; ++i)
        {
            if (!NearlyZero(vector.Data[i], tolerance))
            {
                return false;
            }
        }

        return true;
    }

    template<CFloating T, usize R, usize C>
    constexpr bool NearlyEqual(
        const Matrix<T, R, C>& lhs, const Matrix<T, R, C>& rhs, T tolerance = Epsilon<T>
    )
    {
        for (usize i = 0; i < R * C; ++i)
        {
            if (!NearlyEqual(lhs.Data[i], rhs.Data[i], tolerance))
            {
                return false;
            }
        }

        return true;
    }

    template<CInteger T, usize R, usize C>
    constexpr bool NearlyEqual(const Matrix<T, R, C>& lhs, const Matrix<T, R, C>& rhs)
    {
        for (usize i = 0; i < R * C; ++i)
        {
            if (lhs.Data[i] != rhs.Data[i])
            {
                return false;
            }
        }

        return true;
    }

    constexpr float Radians(float degrees)
    {
        return degrees * 0.01745f;
    }

    constexpr float Degrees(float radians)
    {
        return radians * 57.2958f;
    }
}
