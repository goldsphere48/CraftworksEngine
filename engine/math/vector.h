#pragma once

#include "core/concepts.h"
#include "core/templates.h"
#include "core/types.h"

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
    constexpr AccumulatorT<T> LengthSquared(const Vector<T, N>& vector)
    {
        using Accumulator = AccumulatorT<T>;
        Accumulator result{};

        for (usize i = 0; i < N; ++i)
        {
            const Accumulator value = static_cast<Accumulator>(vector.Data[i]);

            result += value * value;
        }

        return result;
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
    constexpr AccumulatorT<T> DistanceSquared(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        using Accumulator = AccumulatorT<T>;

        Accumulator result{};

        for (usize i = 0; i < N; ++i)
        {
            const Accumulator difference =
                static_cast<Accumulator>(rhs.Data[i]) - static_cast<Accumulator>(lhs.Data[i]);

            result += difference * difference;
        }

        return result;
    }

    template<CScalar T, usize N>
    constexpr Vector<float, N> Normalize(const Vector<T, N>& vector)
    {
        double           length = Length(vector);
        Vector<float, N> result{};
        for (usize i = 0; i < N; ++i)
        {
            result.Data[i] = static_cast<float>(vector.Data[i]) / length;
        }

        return result;
    }

    template<CScalar T, usize N>
    constexpr AccumulatorT<T> Dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        using Result = AccumulatorT<T>;

        Result result = 0;

        for (usize i = 0; i < N; ++i)
        {
            result += static_cast<Result>(lhs.Data[i]) * static_cast<Result>(rhs.Data[i]);
        }

        return result;
    }

    template<CScalar T>
    constexpr AccumulatorT<T> Cross(const Vector<T, 2>& lhs, const Vector<T, 2>& rhs)
    {
        using R = AccumulatorT<T>;

        const R ax = static_cast<R>(lhs.X);
        const R ay = static_cast<R>(lhs.Y);
        const R bx = static_cast<R>(rhs.X);
        const R by = static_cast<R>(rhs.Y);

        return ax * by - ay * bx;
    }

    template<CScalar T>
    constexpr Vector<AccumulatorT<T>, 3> Cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs)
    {
        using R = AccumulatorT<T>;

        const R ax = static_cast<R>(lhs.X);
        const R ay = static_cast<R>(lhs.Y);
        const R az = static_cast<R>(lhs.Z);

        const R bx = static_cast<R>(rhs.X);
        const R by = static_cast<R>(rhs.Y);
        const R bz = static_cast<R>(rhs.Z);

        return Vector<R, 3>{ay * bz - az * by, az * bx - ax * bz, ax * by - ay * bx};
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
}
