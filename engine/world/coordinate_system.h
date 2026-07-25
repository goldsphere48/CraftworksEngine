#pragma once

#include "math/vector.h"

namespace cw::world
{
    inline constexpr math::Vec3 Right
    {
        .Data = {1.0f, 0.0f, 0.0f}
    };

    inline constexpr math::Vec3 Left
    {
        .Data = {-1.0f, 0.0f, 0.0f}
    };

    inline constexpr math::Vec3 Up
    {
        .Data = {0.0f, 1.0f, 0.0f}
    };

    inline constexpr math::Vec3 Down
    {
        .Data = {0.0f, -1.0f, 0.0f}
    };

    inline constexpr math::Vec3 Forward
    {
        .Data = {0.0f, 0.0f, -1.0f}
    };

    inline constexpr math::Vec3 Back
    {
        .Data = {0.0f, 0.0f, 1.0f}
    };
}
