#pragma once

#include "core/concepts.h"
#include "core/templates.h"
#include "core/types.h"
#include "debug/debug.h"
#include "math_utils.h"

#include <cmath>
#include <type_traits>

namespace cw::math
{
    template<CScalar T, usize N>
    requires(N > 0) struct Vector
    {
        T Data[N];
    };

    template<CScalar T>
    struct Vector<T, 2>
    {
        union
        {
            T Data[2];
            struct
            {
                T X;
                T Y;
            };
            struct
            {
                T R;
                T G;
            };
        };
    };

    template<CScalar T>
    struct Vector<T, 3>
    {
        union
        {
            T Data[3];
            struct
            {
                T X;
                T Y;
                T Z;
            };
            struct
            {
                T R;
                T G;
                T B;
            };
        };
    };

    template<CScalar T>
    struct Vector<T, 4>
    {
        union
        {
            T Data[4];
            struct
            {
                T X;
                T Y;
                T Z;
                T W;
            };
            struct
            {
                T R;
                T G;
                T B;
                T A;
            };
        };
    };

    using Vec2f = Vector<float, 2>;
    using Vec3f = Vector<float, 3>;
    using Vec4f = Vector<float, 4>;

    using Vec2i = Vector<int32, 2>;
    using Vec3i = Vector<int32, 3>;
    using Vec4i = Vector<int32, 4>;

    using Vec2 = Vec2f;
    using Vec3 = Vec3f;
    using Vec4 = Vec4f;

    template<CScalar T, usize N>
    constexpr Vector<T, N>& operator+=(Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        for (usize i = 0; i < N; ++i)
        {
            lhs.Data[i] += rhs.Data[i];
        }

        return lhs;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N>& operator-=(Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        for (usize i = 0; i < N; ++i)
        {
            lhs.Data[i] -= rhs.Data[i];
        }

        return lhs;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N>& operator*=(Vector<T, N>& lhs, T rhs)
    {
        for (usize i = 0; i < N; ++i)
        {
            lhs.Data[i] *= rhs;
        }

        return lhs;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N>& operator/=(Vector<T, N>& lhs, T rhs)
    {
        CW_ASSERT(rhs != T{0});

        for (usize i = 0; i < N; ++i)
        {
            lhs.Data[i] /= rhs;
        }

        return lhs;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N> operator+(Vector<T, N> lhs, const Vector<T, N>& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N> operator-(Vector<T, N> lhs, const Vector<T, N>& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N> operator*(Vector<T, N> lhs, T rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N> operator/(Vector<T, N> lhs, T rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    template<CScalar T>
    using LengthResultT = std::conditional_t<CFloating<T>, T, double>;
}

namespace cw::math::vec
{
    template<CScalar T, typename... Args>
    requires(
        sizeof...(Args) > 0 && (std::same_as<std::remove_cvref_t<Args>, T> && ...)
    ) constexpr auto Make(Args&&... args)
    {
        return Vector<T, sizeof...(Args)>{ args... };
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N> Fill(T value)
    {
        Vector<T, N> result{};

        for (usize i = 0; i < N; ++i)
        {
            result.Data[i] = value;
        }

        return result;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N> Axis(usize index, T value)
    {
        Vector<T, N> result{};

        result.Data[index] = value;

        return result;
    }

    template<CScalar T, usize N>
    constexpr Vector<T, N> Zero = Fill<T, N>(T{0});

    template<CScalar T, usize N>
    constexpr Vector<T, N> One = Fill<T, N>(T{1});

    template<CScalar T, usize N>
    constexpr Vector<T, N> UnitX = Axis<T, N>(0, T{1});

    template<CScalar T, usize N>
    requires(N >= 2) constexpr Vector<T, N> UnitY = Axis<T, N>(1, T{1});

    template<CScalar T, usize N>
    requires(N >= 3) constexpr Vector<T, N> UnitZ = Axis<T, N>(2, T{1});

    template<CScalar T, usize N>
    requires(N >= 4) constexpr Vector<T, N> UnitW = Axis<T, N>(3, T{1});

    template<CScalar T, usize N>
    constexpr LengthResultT<T> Length(const Vector<T, N>& vector)
    {
        double result = 0.0;

        for (usize i = 0; i < N; ++i)
        {
            const double value = static_cast<double>(vector.Data[i]);

            result += value * value;
        }

        return static_cast<LengthResultT<T>>(std::sqrt(result));
    }

    template<CScalar T, usize N>
    constexpr T LengthSquared(const Vector<T, N>& vector)
    {
        using Accumulator = AccumulatorT<T>;
        Accumulator result{};

        for (usize i = 0; i < N; ++i)
        {
            const Accumulator value = static_cast<Accumulator>(vector.Data[i]);

            result += value * value;
        }

        return static_cast<T>(result);
    }

    template<CFloating T, usize N>
    constexpr auto Distance(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        return Length(rhs - lhs);
    }

    template<CInteger T, usize N>
    constexpr double Distance(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        double result{};

        for (usize i = 0; i < N; ++i)
        {
            const double difference =
                static_cast<double>(rhs.Data[i]) - static_cast<double>(lhs.Data[i]);

            result += difference * difference;
        }

        return std::sqrt(result);
    }

    template<CFloating T, usize N>
    constexpr auto DistanceSquared(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        return LengthSquared(rhs - lhs);
    }

    template<CInteger T, usize N>
    constexpr T DistanceSquared(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        using Accumulator = AccumulatorT<T>;

        Accumulator result{};

        for (usize i = 0; i < N; ++i)
        {
            const Accumulator difference =
                static_cast<Accumulator>(rhs.Data[i]) - static_cast<Accumulator>(lhs.Data[i]);

            result += difference * difference;
        }

        return static_cast<T>(result);
    }

    template<CScalar T, usize N>
    constexpr Vector<float, N> Normalize(const Vector<T, N>& vector)
    {
        const double length = static_cast<double>(Length(vector));

        CW_ASSERT(length != 0.0);

        Vector<float, N> result{};
        for (usize i = 0; i < N; ++i)
        {
            result.Data[i] = static_cast<float>(static_cast<double>(vector.Data[i]) / length);
        }

        return result;
    }

    template<CScalar T, usize N>
    constexpr T Dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        using Accumulator = AccumulatorT<T>;

        Accumulator result{};

        for (usize i = 0; i < N; ++i)
        {
            result +=
                static_cast<Accumulator>(lhs.Data[i]) * static_cast<Accumulator>(rhs.Data[i]);
        }

        return static_cast<T>(result);
    }

    template<CScalar T>
    constexpr T Cross(const Vector<T, 2>& lhs, const Vector<T, 2>& rhs)
    {
        using Accumulator = AccumulatorT<T>;

        const Accumulator ax = static_cast<Accumulator>(lhs.X);
        const Accumulator ay = static_cast<Accumulator>(lhs.Y);
        const Accumulator bx = static_cast<Accumulator>(rhs.X);
        const Accumulator by = static_cast<Accumulator>(rhs.Y);

        return static_cast<T>(ax * by - ay * bx);
    }

    template<CScalar T>
    constexpr Vector<T, 3> Cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs)
    {
        using Accumulator = AccumulatorT<T>;

        const Accumulator ax = static_cast<Accumulator>(lhs.X);
        const Accumulator ay = static_cast<Accumulator>(lhs.Y);
        const Accumulator az = static_cast<Accumulator>(lhs.Z);

        const Accumulator bx = static_cast<Accumulator>(rhs.X);
        const Accumulator by = static_cast<Accumulator>(rhs.Y);
        const Accumulator bz = static_cast<Accumulator>(rhs.Z);

        return Vector<T, 3>{
            static_cast<T>(ay * bz - az * by),
            static_cast<T>(az * bx - ax * bz),
            static_cast<T>(ax * by - ay * bx)
        };
    }

    template<CScalar T>
    constexpr Vector<T, 2> PerpCCW(const Vector<T, 2>& vector)
    {
        return {-vector.Y, vector.X};
    }

    template<CScalar T>
    constexpr Vector<T, 2> PerpCW(const Vector<T, 2>& vector)
    {
        return {vector.Y, -vector.X};
    }
}

namespace cw::math::vec2
{
    inline constexpr Vec2 Zero  = vec::Zero<float, 2>;
    inline constexpr Vec2 One   = vec::One<float, 2>;
    inline constexpr Vec2 UnitX = vec::UnitX<float, 2>;
    inline constexpr Vec2 UnitY = vec::UnitY<float, 2>;
}

namespace cw::math::vec3
{
    inline constexpr Vec3 Zero  = vec::Zero<float, 3>;
    inline constexpr Vec3 One   = vec::One<float, 3>;
    inline constexpr Vec3 UnitX = vec::UnitX<float, 3>;
    inline constexpr Vec3 UnitY = vec::UnitY<float, 3>;
    inline constexpr Vec3 UnitZ = vec::UnitZ<float, 3>;
}

namespace cw::math::vec4
{
    inline constexpr Vec4 Zero  = vec::Zero<float, 4>;
    inline constexpr Vec4 One   = vec::One<float, 4>;
    inline constexpr Vec4 UnitX = vec::UnitX<float, 4>;
    inline constexpr Vec4 UnitY = vec::UnitY<float, 4>;
    inline constexpr Vec4 UnitZ = vec::UnitZ<float, 4>;
    inline constexpr Vec4 UnitW = vec::UnitW<float, 4>;
}
