#pragma once

#include "quat.h"
#include "vector.h"

namespace cw::math
{
    struct Transform
    {
        Vec3       Position = vec3::Zero<float, 3>;
        Quaternion Rotation = qaut::Identity;
        Vec3       Scale    = vec3::One<float, 3>;
    };
}
