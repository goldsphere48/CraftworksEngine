#pragma once

#include "math/matrix.h"
#include "math/vector.h"
#include "math/quat.h"

namespace cw::math::mat4
{
    constexpr Vec3 TransformDirection(const Mat4& matrix, const Vec3& direction)
    {
        Vec4 transformed = matrix * Vec4 { direction.X, direction.Y, direction.Z, 0.0f };
        
        return Vec3
        {
            transformed.X,
            transformed.Y,
            transformed.Z
        };
    }

    constexpr Vec3 TransformPoint(const Mat4& matrix, const Vec3& point)
    {
        Vec4 transformed = matrix * Vec4 { point.X, point.Y, point.Z, 1.0f };
        
        return Vec3
        {
            transformed.X,
            transformed.Y,
            transformed.Z
        }; 
    }
    
    constexpr Vec3 ProjectPoint(const Mat4& matrix, const Vec3& point)
    {
        Vec4 transformed = matrix * Vec4 { point.X, point.Y, point.Z, 1.0f };

        float w = transformed.W;

        CW_ASSERT(w != 0.0f);
        
        return Vec3
        {
            transformed.X / w,
            transformed.Y / w,
            transformed.Z / w
        };
    }

    constexpr Mat4 Translation(const Vec3& vector)
    {
        return mat::Make<float, 4, 4>(
            1.0f, 0.0f, 0.0f, vector.X,
            0.0f, 1.0f, 0.0f, vector.Y,
            0.0f, 0.0f, 1.0f, vector.Z,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    constexpr Mat4 Scale(const Vec3& scale)
    {
        return mat::Make<float, 4, 4>(
            scale.X, 0.0f, 0.0f, 0.0f,
            0.0f, scale.Y, 0.0f, 0.0f,
            0.0f, 0.0f, scale.Z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }
    
    constexpr Mat4 Scale(float scale)
    {
        return Scale(Vec3{
            scale,
            scale,
            scale
        });
    }

    inline Mat4 RotationZ(float radians)
    {
        const float sina = std::sin(radians);
        const float cosa = std::cos(radians);

        return mat::Make<float, 4, 4>(
            cosa, -sina, 0.0f, 0.0f,
            sina,  cosa, 0.0f, 0.0f,
            0.0f,  0.0f, 1.0f, 0.0f,
            0.0f,  0.0f, 0.0f, 1.0f
        );
    }

    inline Mat4 RotationX(float radians)
    {
        const float sina = std::sin(radians);
        const float cosa = std::cos(radians);

        return mat::Make<float, 4, 4>(
            1.0f, 0.0f,  0.0f, 0.0f,
            0.0f, cosa, -sina, 0.0f,
            0.0f, sina,  cosa, 0.0f,
            0.0f, 0.0f,  0.0f, 1.0f
        );
    }

    inline Mat4 RotationY(float radians)
    {
        const float sina = std::sin(radians);
        const float cosa = std::cos(radians);

        return mat::Make<float, 4, 4>(
             cosa, 0.0f, sina, 0.0f,
             0.0f, 1.0f, 0.0f, 0.0f,
            -sina, 0.0f, cosa, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f
        );
    }
    
    inline Mat4 RotateEuler(float x, float y, float z)
    {
        return RotationZ(z) * RotationY(y) * RotationX(x);
    }

    constexpr Mat4 TRS(const Vec3& translation, const Quaternion& rotation, const Vec3& scale)
    {
        return Translation(translation) * quat::ToMat4(rotation) * Scale(scale);
    }
}

namespace cw::math::cam
{
    constexpr Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
    {
        const Vec3 back = vec::Normalize(eye - target);
        const Vec3 right = vec::Normalize(vec::Cross(up, back));
        const Vec3 cameraUp = vec::Cross(back, right);

        return mat::Make<float, 4, 4>(
            right.X,    right.Y,    right.Z,    -vec::Dot(right, eye),
            cameraUp.X, cameraUp.Y, cameraUp.Z, -vec::Dot(cameraUp, eye),
            back.X,     back.Y,     back.Z,     -vec::Dot(back, eye),
            0.0f,       0.0f,       0.0f,        1.0f
        );
    }

    inline Mat4 PerspectiveRH_ZO(
        float verticalFov,
        float aspectRatio,
        float nearPlane,
        float farPlane
    )
    {
        CW_ASSERT(verticalFov > 0.0f);
        CW_ASSERT(verticalFov < Pi);
        CW_ASSERT(aspectRatio > 0.0f);
        CW_ASSERT(nearPlane > 0.0f);
        CW_ASSERT(farPlane > nearPlane);

        float yScale = 1.0f / std::tan(verticalFov * 0.5f);
        float xScale = yScale / aspectRatio;
        const float depth = farPlane - nearPlane;

        return mat::Make<float, 4, 4>(
            xScale, 0.0f,   0.0f,              0.0f,
            0.0f,   yScale, 0.0f,              0.0f,
            0.0f,   0.0f,  -farPlane / depth, -(farPlane * nearPlane) / depth,
            0.0f,   0.0f,  -1.0f,              0.0f
        );
    }

    inline Mat4 PerspectiveRH_NO(
        float verticalFov,
        float aspectRatio,
        float nearPlane,
        float farPlane
    )
    {
        CW_ASSERT(verticalFov > 0.0f);
        CW_ASSERT(verticalFov < Pi);
        CW_ASSERT(aspectRatio > 0.0f);
        CW_ASSERT(nearPlane > 0.0f);
        CW_ASSERT(farPlane > nearPlane);

        float yScale = 1.0f / std::tan(verticalFov * 0.5f);
        float xScale = yScale / aspectRatio;
        const float depth = farPlane - nearPlane;

        return mat::Make<float, 4, 4>(
            xScale, 0.0f,   0.0f,                              0.0f,
            0.0f,   yScale, 0.0f,                              0.0f,
            0.0f,   0.0f,  -(farPlane + nearPlane) / depth,   -(2.0f * farPlane * nearPlane) / depth,
            0.0f,   0.0f,  -1.0f,                              0.0f
        );
    }

    inline Mat4 OrthographicRH_NO(
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    )
    {
        CW_ASSERT(right != left);
        CW_ASSERT(top != bottom);
        CW_ASSERT(farPlane > nearPlane);
        CW_ASSERT(nearPlane > 0.0f);

        const float width  = right - left;
        const float height = top - bottom;
        const float depth  = farPlane - nearPlane;

        return mat::Make<float, 4, 4>(
            2.0f / width, 0.0f,          0.0f,         -(right + left) / width,
            0.0f,         2.0f / height, 0.0f,         -(top + bottom) / height,
            0.0f,         0.0f,         -2.0f / depth, -(farPlane + nearPlane) / depth,
            0.0f,         0.0f,          0.0f,          1.0f
        );
    }

    inline Mat4 OrthographicRH_ZO(
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    )
    {
        CW_ASSERT(right != left);
        CW_ASSERT(top != bottom);
        CW_ASSERT(farPlane > nearPlane);
        CW_ASSERT(nearPlane > 0.0f);

        const float width  = right - left;
        const float height = top - bottom;
        const float depth  = farPlane - nearPlane;

        return mat::Make<float, 4, 4>(
            2.0f / width, 0.0f,          0.0f,         -(right + left) / width,
            0.0f,         2.0f / height, 0.0f,         -(top + bottom) / height,
            0.0f,         0.0f,         -1.0f / depth, -nearPlane / depth,
            0.0f,         0.0f,          0.0f,          1.0f
        );
    }
}
