#pragma once

#include "debug/debug.h"
#include "math_utils.h"
#include "vector.h"
#include "matrix.h"

#include <algorithm>
#include <cmath>

namespace cw::math
{
    struct Quaternion
    {
        union
        {
            float Data[4];

            struct
            {
                float X;
                float Y;
                float Z;
                float W;
            };
        };
    };

    constexpr Quaternion operator*(const Quaternion& quat, float scalar)
    {
        return
        {
            quat.X * scalar,
            quat.Y * scalar,
            quat.Z * scalar,
            quat.W * scalar
        };
    }

    constexpr Quaternion operator*(float scalar, const Quaternion& quat)
    {
        return quat * scalar;
    }

    constexpr Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
    {
        const Vec3 lhsVector{
            lhs.X,
            lhs.Y,
            lhs.Z
        };

        const Vec3 rhsVector{
            rhs.X,
            rhs.Y,
            rhs.Z
        };

        const Vec3 vectorPart =
            rhsVector * lhs.W + lhsVector * rhs.W + vec::Cross(lhsVector, rhsVector);
        const float scalarPart = lhs.W * rhs.W - vec::Dot(lhsVector, rhsVector);

        return Quaternion
        {
            vectorPart.X,
            vectorPart.Y,
            vectorPart.Z,
            scalarPart
        };
    }

    constexpr Quaternion operator/(const Quaternion& quat, float scalar)
    {
        CW_ASSERT(scalar != 0.0f);

        return
        {
            quat.X / scalar,
            quat.Y / scalar,
            quat.Z / scalar,
            quat.W / scalar
        };
    }

    constexpr Quaternion operator-(const Quaternion& quat)
    {
        return
        {
            -quat.X,
            -quat.Y,
            -quat.Z,
            -quat.W
        };
    }

    constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)
    {
        return {
            lhs.X + rhs.X,
            lhs.Y + rhs.Y,
            lhs.Z + rhs.Z,
            lhs.W + rhs.W
        };
    }

    constexpr Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs)
    {
        return {
            lhs.X - rhs.X,
            lhs.Y - rhs.Y,
            lhs.Z - rhs.Z,
            lhs.W - rhs.W
        };
    }
}

namespace cw::math::quat
{
    inline constexpr Quaternion Identity = {
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    constexpr float LengthSquared(const Quaternion& quat)
    {
        return
            quat.X * quat.X +
            quat.Y * quat.Y +
            quat.Z * quat.Z +
            quat.W * quat.W;
    }
    
    constexpr bool IsNormalized(const Quaternion& quat, float tolerance = 1e-5f)
    {
        return Abs(LengthSquared(quat) - 1.0f) <= tolerance;
    }

    inline float Length(const Quaternion& quat)
    {
        return std::sqrt(LengthSquared(quat));
    }

    inline Quaternion Normalize(const Quaternion& quat)
    {
        const float length = Length(quat);

        CW_ASSERT(length != 0.0f);

        return Quaternion { quat.X / length, quat.Y / length, quat.Z / length, quat.W / length };
    }

    constexpr Quaternion Conjugate(const Quaternion& quat)
    {
        return Quaternion { -quat.X, -quat.Y, -quat.Z, quat.W };
    }

    constexpr Quaternion Inverse(const Quaternion& quat)
    {
        const float lengthSquared = LengthSquared(quat);

        CW_ASSERT(lengthSquared != 0.0f);

        return
        {
            -quat.X / lengthSquared,
            -quat.Y / lengthSquared,
            -quat.Z / lengthSquared,
             quat.W / lengthSquared
        };
    }

    constexpr float Dot(const Quaternion& lhs, const Quaternion& rhs)
    {
        return
            lhs.X * rhs.X +
            lhs.Y * rhs.Y +
            lhs.Z * rhs.Z +
            lhs.W * rhs.W;
    }

    inline Quaternion FromAxisAngle(const Vec3& axis, float angle)
    {
        const float halfAngle = angle * 0.5f;
        const Vec3  vector    = vec::Normalize(axis) * std::sin(halfAngle);

        return
        {
            vector.X,
            vector.Y,
            vector.Z,
            std::cos(halfAngle)
        };
    }

    constexpr Vec3 RotateVector(const Quaternion& quat, const Vec3& vector)
    {
        CW_ASSERT(IsNormalized(quat));

        const Quaternion point    = { vector.X, vector.Y, vector.Z, 0.0f };
        const Quaternion rotated  = quat * point * Conjugate(quat);

        return
        {
            rotated.X,
            rotated.Y,
            rotated.Z
        };
    }

    constexpr Mat3 ToMat3(const Quaternion& quat)
    {
        CW_ASSERT(IsNormalized(quat));

        return mat::Make<float, 3, 3>(
            1.0f - 2.0f * quat.Y * quat.Y - 2.0f * quat.Z * quat.Z,
                   2.0f * quat.X * quat.Y - 2.0f * quat.W * quat.Z,
                   2.0f * quat.X * quat.Z + 2.0f * quat.W * quat.Y,

                   2.0f * quat.X * quat.Y + 2.0f * quat.W * quat.Z,
            1.0f - 2.0f * quat.X * quat.X - 2.0f * quat.Z * quat.Z,
                   2.0f * quat.Y * quat.Z - 2.0f * quat.W * quat.X,

                   2.0f * quat.X * quat.Z - 2.0f * quat.W * quat.Y,
                   2.0f * quat.Y * quat.Z + 2.0f * quat.W * quat.X,
            1.0f - 2.0f * quat.X * quat.X - 2.0f * quat.Y * quat.Y
        );
    }

    constexpr Mat4 ToMat4(const Quaternion& quat)
    {
        CW_ASSERT(IsNormalized(quat));

        const float xx = quat.X * quat.X;
        const float yy = quat.Y * quat.Y;
        const float zz = quat.Z * quat.Z;

        const float xy = quat.X * quat.Y;
        const float xz = quat.X * quat.Z;
        const float yz = quat.Y * quat.Z;

        const float wx = quat.W * quat.X;
        const float wy = quat.W * quat.Y;
        const float wz = quat.W * quat.Z;

        return mat::Make<float, 4, 4>(
            1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz),        2.0f * (xz + wy),        0.0f,
            2.0f * (xy + wz),        1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx),        0.0f,
            2.0f * (xz - wy),        2.0f * (yz + wx),        1.0f - 2.0f * (xx + yy), 0.0f,
            0.0f,                    0.0f,                    0.0f,                    1.0f
        );
    }

    inline Quaternion Euler(float x, float y, float z)
    {
        const float halfX = x * 0.5f;
        const float halfY = y * 0.5f;
        const float halfZ = z * 0.5f;

        const float cx = std::cos(halfX);
        const float cy = std::cos(halfY);
        const float cz = std::cos(halfZ);
        const float sx = std::sin(halfX);
        const float sy = std::sin(halfY);
        const float sz = std::sin(halfZ);

        return
        {
            sx * cy * cz - cx * sy * sz,
            cx * sy * cz + sx * cy * sz,
            cx * cy * sz - sx * sy * cz,
            cx * cy * cz + sx * sy * sz
        };
    }

    inline Quaternion Slerp(const Quaternion& q0, Quaternion q1, float t)
    {
        CW_ASSERT(IsNormalized(q0));
        CW_ASSERT(IsNormalized(q1));

        float cosine = Dot(q0, q1);

        if (cosine < 0.0f)
        {
            cosine = -cosine;
            q1 = -q1;
        }

        cosine = std::clamp(cosine, -1.0f, 1.0f);

        constexpr float LinearThreshold = 0.9995f;

        if (cosine > LinearThreshold)
        {
            return Normalize(
                q0 * (1.0f - t) +
                q1 * t
                );
        }

        const float angle = std::acos(cosine);
        const float sine = std::sin(angle);

        const float q0Weight =
        std::sin((1.0f - t) * angle) / sine;

        const float q1Weight =
        std::sin(t * angle) / sine;

        return Normalize(
            q0 * q0Weight +
            q1 * q1Weight
        );
    }
}
