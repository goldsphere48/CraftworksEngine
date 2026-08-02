#include <gtest/gtest.h>

#include <math/vector.h>
#include <math/matrix.h>
#include <world/coordinate_system.h>

using namespace cw;
using namespace cw::math;

TEST(MathTransforms, TransformPointDoesntAffectDirection)
{
    const Mat4 translation = mat::Make<float, 4, 4>(
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    const Vec3 point{2.0f, 3.0f, 4.0f};
    const Vec3 direction{2.0f, 3.0f, 4.0f};

    const Vec3 transformedPoint =
        mat4::TransformPoint(translation, point);

    const Vec3 transformedDirection =
        mat4::TransformDirection(translation, direction);

    EXPECT_FLOAT_EQ(transformedDirection.X, 2.0f);
    EXPECT_FLOAT_EQ(transformedDirection.Y, 3.0f);
    EXPECT_FLOAT_EQ(transformedDirection.Z, 4.0f);

    EXPECT_FLOAT_EQ(transformedPoint.X, 3.0f);
    EXPECT_FLOAT_EQ(transformedPoint.Y, 4.0f);
    EXPECT_FLOAT_EQ(transformedPoint.Z, 5.0f);
}
