#pragma once

#include "core/concepts.h"
#include "core/templates.h"
#include "core/types.h"
#include "debug/debug.h"
#include "math_utils.h"
#include "vector.h"

#include <utility>
#include <type_traits>

namespace cw::math
{
    template<CScalar T, usize R, usize C>
    requires(R > 0 && C > 0)
    struct Matrix
    {
        T Data[R * C];
    };

    using Mat2  = Matrix<float, 2, 2>;
    using Mat3  = Matrix<float, 3, 3>;
    using Mat4  = Matrix<float, 4, 4>;
    
    using Mat32 = Matrix<float, 3, 2>;
    using Mat34 = Matrix<float, 3, 4>;

    template<CScalar T>
    using DeterminantResultT = std::conditional_t<CFloating<T>, T, int64>;

    namespace mat
    {
        template<CScalar T, usize R, usize C>
        constexpr const T& At(const Matrix<T, R, C>& matrix, usize row, usize column)
        {
            CW_ASSERT(row < R && column < C);

            return matrix.Data[column * R + row];
        }

        template<CScalar T, usize R, usize C>
        constexpr T& At(Matrix<T, R, C>& matrix, usize row, usize column)
        {
            CW_ASSERT(row < R && column < C);

            return matrix.Data[column * R + row];
        }
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C>& operator*=(Matrix<T, R, C>& matrix, T scalar)
    {
        for (usize i = 0; i < R * C; ++i)
        {
            matrix.Data[i] *= scalar;
        }

        return matrix;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C>& operator/=(Matrix<T, R, C>& matrix, T scalar)
    {
        CW_ASSERT(scalar != T{0});

        for (usize i = 0; i < R * C; ++i)
        {
            matrix.Data[i] /= scalar;
        }

        return matrix;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C>& operator+=(Matrix<T, R, C>& lhs, const Matrix<T, R, C>& rhs)
    {
        for (usize i = 0; i < R * C; ++i)
        {
            lhs.Data[i] += rhs.Data[i];
        }

        return lhs;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C>& operator-=(Matrix<T, R, C>& lhs, const Matrix<T, R, C>& rhs)
    {
        for (usize i = 0; i < R * C; ++i)
        {
            lhs.Data[i] -= rhs.Data[i];
        }

        return lhs;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C> operator+(Matrix<T, R, C> lhs, const Matrix<T, R, C>& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C> operator-(Matrix<T, R, C> lhs, const Matrix<T, R, C>& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C> operator*(Matrix<T, R, C> lhs, T scalar)
    {
        lhs *= scalar;
        return lhs;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C> operator*(T scalar, Matrix<T, R, C> lhs)
    {
        lhs *= scalar;
        return lhs;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, R, C> operator/(Matrix<T, R, C> lhs, T scalar)
    {
        lhs /= scalar;
        return lhs;
    }

    template<CScalar T, usize R, usize C>
    constexpr Vector<T, R> operator*(const Matrix<T, R, C>& lhs, const Vector<T, C>& vector)
    {
        Vector<T, R> result{};
        for (usize row = 0; row < R; ++row)
        {
            for (usize col = 0; col < C; ++col)
            {
                result.Data[row] += mat::At(lhs, row, col) * vector.Data[col];
            }
        }

        return result;
    }

    template<CScalar T, usize R, usize C, usize K>
    constexpr Matrix<T, R, C> operator*(const Matrix<T, R, K>& lhs, const Matrix<T, K, C>& rhs)
    {
        Matrix<T, R, C> result{};
        for (usize row = 0; row < R; ++row)
        {
            for (usize col = 0; col < C; ++col)
            {
                T value{};
                for (usize k = 0; k < K; ++k)
                {
                    value += mat::At(lhs, row, k) * mat::At(rhs, k, col);
                }

                mat::At(result, row, col) = value;
            }
        }

        return result;
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
}

namespace cw::math::mat
{
    template<CScalar T, usize R, usize C, typename... Args>
    requires(
        sizeof...(Args) == R * C && (std::same_as<std::remove_cvref_t<Args>, T> && ...)
    ) constexpr Matrix<T, R, C> Make(Args... args)
    {
        const T values[R * C]{args...};

        Matrix<T, R, C> matrix{};

        for (usize row = 0; row < R; ++row)
        {
            for (usize column = 0; column < C; ++column)
            {
                matrix.Data[column * R + row] = values[row * C + column];
            }
        }

        return matrix;
    }

    template<CScalar T, usize R, usize C>
    constexpr Vector<T, C> GetRow(const Matrix<T, R, C>& matrix, usize row)
    {
        CW_ASSERT(row < R);

        Vector<T, C> vector{};

        for (usize i = 0; i < C; ++i)
        {
            vector.Data[i] = At(matrix, row, i);
        }

        return vector;
    }

    template<CScalar T, usize R, usize C>
    constexpr Vector<T, R> GetColumn(const Matrix<T, R, C>& matrix, usize column)
    {
        CW_ASSERT(column < C);

        Vector<T, R> vector{};

        for (usize i = 0; i < R; ++i)
        {
            vector.Data[i] = At(matrix, i, column);
        }

        return vector;
    }

    template<CScalar T, usize R, usize C>
    constexpr void SetRow(Matrix<T, R, C>& matrix, usize row, const Vector<T, C>& vector)
    {
        CW_ASSERT(row < R);

        for (usize i = 0; i < C; ++i)
        {
            At(matrix, row, i) = vector.Data[i];
        }
    }

    template<CScalar T, usize R, usize C>
    constexpr void SetColumn(Matrix<T, R, C>& matrix, usize column, const Vector<T, R>& vector)
    {
        CW_ASSERT(column < C);

        for (usize i = 0; i < R; ++i)
        {
            At(matrix, i, column) = vector.Data[i];
        }
    }

    template<CScalar T, usize R, typename... Columns>
    requires(
        sizeof...(Columns) > 0 && (std::same_as<std::remove_cvref_t<Columns>, Vector<T, R>> && ...)
    ) constexpr auto FromColumns(const Columns&... columns)
    {
        constexpr usize C = sizeof...(Columns);

        Matrix<T, R, C> matrix{};

        const Vector<T, R> values[C]{columns...};

        for (usize column = 0; column < C; ++column)
        {
            SetColumn(matrix, column, values[column]);
        }

        return matrix;
    }

    template<CScalar T, usize C, typename... Rows>
    requires(
        sizeof...(Rows) > 0 && (std::same_as<std::remove_cvref_t<Rows>, Vector<T, C>> && ...)
    ) constexpr auto FromRows(const Rows&... rows)
    {
        constexpr usize R = sizeof...(Rows);

        Matrix<T, R, C> matrix{};

        const Vector<T, C> values[R]{rows...};

        for (usize row = 0; row < R; ++row)
        {
            SetRow(matrix, row, values[row]);
        }

        return matrix;
    }

    template<CScalar T, usize R, usize C>
    constexpr Matrix<T, C, R> Transpose(const Matrix<T, R, C>& matrix)
    {
        Matrix<T, C, R> result{};
        for (usize row = 0; row < R; ++row)
        {
            for (usize col = 0; col < C; ++col)
            {
                At(result, col, row) = At(matrix, row, col);
            }
        }

        return result;
    }

    template<CScalar T, usize R, usize C>
    inline constexpr Matrix<T, R, C> Zero{};

    template<CScalar T, usize N>
    inline constexpr Matrix<T, N, N> Identity = []
    {
        Matrix<T, N, N> result{};

        for (usize i = 0; i < N; ++i)
        {
            result.Data[i * N + i] = T{1};
        }

        return result;
    }();

    template<CScalar T>
    constexpr DeterminantResultT<T> Determinant(const Matrix<T, 1, 1>& matrix)
    {
        return static_cast<DeterminantResultT<T>>(At(matrix, 0, 0));
    }

    template<CScalar T>
    constexpr DeterminantResultT<T> Determinant(const Matrix<T, 2, 2>& matrix)
    {
        using R = DeterminantResultT<T>;

        const R a = static_cast<R>(At(matrix, 0, 0));
        const R b = static_cast<R>(At(matrix, 0, 1));

        const R c = static_cast<R>(At(matrix, 1, 0));
        const R d = static_cast<R>(At(matrix, 1, 1));

        return a * d - b * c;
    }

    template<CScalar T>
    constexpr DeterminantResultT<T> Determinant(const Matrix<T, 3, 3>& matrix)
    {
        using R = DeterminantResultT<T>;

        const R a = static_cast<R>(At(matrix, 0, 0));
        const R b = static_cast<R>(At(matrix, 0, 1));
        const R c = static_cast<R>(At(matrix, 0, 2));

        const R d = static_cast<R>(At(matrix, 1, 0));
        const R e = static_cast<R>(At(matrix, 1, 1));
        const R f = static_cast<R>(At(matrix, 1, 2));

        const R g = static_cast<R>(At(matrix, 2, 0));
        const R h = static_cast<R>(At(matrix, 2, 1));
        const R i = static_cast<R>(At(matrix, 2, 2));

        return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    }

    template<CScalar T>
    constexpr DeterminantResultT<T> Determinant(const Matrix<T, 4, 4>& matrix)
    {
        using R = DeterminantResultT<T>;

        const R a = static_cast<R>(At(matrix, 0, 0));
        const R b = static_cast<R>(At(matrix, 0, 1));
        const R c = static_cast<R>(At(matrix, 0, 2));
        const R d = static_cast<R>(At(matrix, 0, 3));

        const R e = static_cast<R>(At(matrix, 1, 0));
        const R f = static_cast<R>(At(matrix, 1, 1));
        const R g = static_cast<R>(At(matrix, 1, 2));
        const R h = static_cast<R>(At(matrix, 1, 3));

        const R i = static_cast<R>(At(matrix, 2, 0));
        const R j = static_cast<R>(At(matrix, 2, 1));
        const R k = static_cast<R>(At(matrix, 2, 2));
        const R l = static_cast<R>(At(matrix, 2, 3));

        const R m = static_cast<R>(At(matrix, 3, 0));
        const R n = static_cast<R>(At(matrix, 3, 1));
        const R o = static_cast<R>(At(matrix, 3, 2));
        const R p = static_cast<R>(At(matrix, 3, 3));

        const R kp_lo = k * p - l * o;
        const R jp_ln = j * p - l * n;
        const R jo_kn = j * o - k * n;

        const R ip_lm = i * p - l * m;
        const R io_km = i * o - k * m;
        const R in_jm = i * n - j * m;

        return a * (f * kp_lo - g * jp_ln + h * jo_kn) - b * (e * kp_lo - g * ip_lm + h * io_km)
               + c * (e * jp_ln - f * ip_lm + h * in_jm) - d * (e * jo_kn - f * io_km + g * in_jm);
    }

    template<CFloating T, usize N>
    constexpr DeterminantResultT<T> Determinant(const Matrix<T, N, N>& source)
    {
        Matrix<T, N, N> matrix      = source;
        T               determinant = T{1};
        bool            negate      = false;

        for (usize pivotColumn = 0; pivotColumn < N; ++pivotColumn)
        {
            usize pivotRow       = pivotColumn;
            T     pivotMagnitude = Abs(At(matrix, pivotColumn, pivotColumn));

            for (usize row = pivotColumn + 1; row < N; ++row)
            {
                const T magnitude = Abs(At(matrix, row, pivotColumn));

                if (magnitude > pivotMagnitude)
                {
                    pivotMagnitude = magnitude;
                    pivotRow       = row;
                }
            }

            if (pivotMagnitude == T{0})
            {
                return T{0};
            }

            if (pivotRow != pivotColumn)
            {
                for (usize column = 0; column < N; ++column)
                {
                    const T temporary = At(matrix, pivotColumn, column);
                    At(matrix, pivotColumn, column) = At(matrix, pivotRow, column);
                    At(matrix, pivotRow, column) = temporary;
                }

                negate = !negate;
            }

            const T pivot = At(matrix, pivotColumn, pivotColumn);

            determinant *= pivot;

            for (usize row = pivotColumn + 1; row < N; ++row)
            {
                const T factor = At(matrix, row, pivotColumn) / pivot;

                for (usize column = pivotColumn + 1; column < N; ++column)
                {
                    At(matrix, row, column) -= factor * At(matrix, pivotColumn, column);
                }
            }
        }

        return negate ? -determinant : determinant;
    }

    template<CScalar R, CScalar T, usize RCount, usize CCount>
    constexpr Matrix<R, RCount, CCount> Cast(const Matrix<T, RCount, CCount>& matrix)
    {
        Matrix<R, RCount, CCount> result{};

        for (usize i = 0; i < RCount * CCount; ++i)
        {
            result.Data[i] = static_cast<R>(matrix.Data[i]);
        }

        return result;
    }

    template<CFloating R, CScalar T>
    constexpr bool TryInverse(const Matrix<T, 2, 2>& matrix, Matrix<R, 2, 2>& result)
    {
        const R a = static_cast<R>(At(matrix, 0, 0));
        const R b = static_cast<R>(At(matrix, 0, 1));
        const R c = static_cast<R>(At(matrix, 1, 0));
        const R d = static_cast<R>(At(matrix, 1, 1));

        const R determinant = a * d - b * c;

        if (determinant == R{0})
        {
            return false;
        }

        const R inverseDeterminant = R{1} / determinant;

        At(result, 0, 0) =  d * inverseDeterminant;
        At(result, 0, 1) = -b * inverseDeterminant;
        At(result, 1, 0) = -c * inverseDeterminant;
        At(result, 1, 1) =  a * inverseDeterminant;

        return true;
    }

    template<CFloating R = float, CScalar T, usize N>
    constexpr bool TryInverse(const Matrix<T, N, N>& source, Matrix<R, N, N>& result)
    {
        Matrix<R, N, N> matrix  = Cast<R>(source);
        Matrix<R, N, N> inverse = mat::Identity<R, N>;

        for (usize pivotColumn = 0; pivotColumn < N; ++pivotColumn)
        {
            usize pivotRow       = pivotColumn;
            R     pivotMagnitude = Abs(At(matrix, pivotRow, pivotColumn));

            for (usize row = pivotColumn + 1; row < N; ++row)
            {
                const R magnitude = Abs(At(matrix, row, pivotColumn));

                if (magnitude > pivotMagnitude)
                {
                    pivotMagnitude = magnitude;
                    pivotRow       = row;
                }
            }

            if (pivotMagnitude == R{0})
            {
                return false;
            }

            if (pivotRow != pivotColumn)
            {
                for (usize column = 0; column < N; ++column)
                {
                    std::swap(At(matrix, pivotRow, column), At(matrix, pivotColumn, column));
                    std::swap(At(inverse, pivotRow, column), At(inverse, pivotColumn, column));
                }
            }

            const R pivot = At(matrix, pivotColumn, pivotColumn);

            for (usize column = 0; column < N; ++column)
            {
                At(matrix, pivotColumn, column) /= pivot;
                At(inverse, pivotColumn, column) /= pivot;
            }

            for (usize row = 0; row < N; ++row)
            {
                if (row == pivotColumn)
                {
                    continue;
                }

                const R factor = At(matrix, row, pivotColumn);

                for (usize column = 0; column < N; ++column)
                {
                    At(matrix, row, column) -= factor * At(matrix, pivotColumn, column);
                    At(inverse, row, column) -= factor * At(inverse, pivotColumn, column);
                }
            }
        }

        result = inverse;

        return true;
    }

    template<CFloating R = float, CScalar T, usize N>
    constexpr Matrix<R, N, N> Inverse(const Matrix<T, N, N>& matrix)
    {
        Matrix<R, N, N> result{};

        const bool success = TryInverse(matrix, result);
        CW_ASSERT(success);

        return result;
    }
}

namespace cw::math::mat2
{
    inline constexpr Mat2 Identity = mat::Identity<float, 2>;
    inline constexpr Mat2 Zero     = mat::Zero<float, 2, 2>;
}

namespace cw::math::mat3
{
    inline constexpr Mat3 Identity = mat::Identity<float, 3>;
    inline constexpr Mat3 Zero     = mat::Zero<float, 3, 3>;
}

namespace cw::math::mat4
{
    inline constexpr Mat4 Identity = mat::Identity<float, 4>;
    inline constexpr Mat4 Zero     = mat::Zero<float, 4, 4>;
}
