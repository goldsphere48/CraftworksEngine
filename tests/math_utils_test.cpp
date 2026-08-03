#include <gtest/gtest.h>

#include <math/math_utils.h>
#include <math/matrix.h>
#include <math/vector.h>

#include <limits>
#include <type_traits>

using namespace cw;
using namespace cw::math;

namespace
{
    constexpr float FloatEpsilon = std::numeric_limits<float>::epsilon();

    constexpr float Infinity = std::numeric_limits<float>::infinity();
    constexpr float NotANumber = std::numeric_limits<float>::quiet_NaN();
}

TEST(MathUtilsAbsTest, DropsTheSign)
{
    EXPECT_FLOAT_EQ(2.5f, Abs(-2.5f));
    EXPECT_FLOAT_EQ(2.5f, Abs(2.5f));
    EXPECT_DOUBLE_EQ(0.5, Abs(-0.5));
    EXPECT_EQ(7, Abs(-7));
    EXPECT_EQ(7, Abs(7));
}

TEST(MathUtilsAbsTest, KeepsZeroZero)
{
    EXPECT_FLOAT_EQ(0.0f, Abs(0.0f));
    EXPECT_FLOAT_EQ(0.0f, Abs(-0.0f));
    EXPECT_EQ(0, Abs(0));
}

TEST(MathUtilsAbsTest, KeepsTheArgumentType)
{
    EXPECT_TRUE((std::is_same_v<decltype(Abs(-1.0f)), float>));
    EXPECT_TRUE((std::is_same_v<decltype(Abs(-1.0)), double>));
    EXPECT_TRUE((std::is_same_v<decltype(Abs(int32{-1})), int32>));
}

TEST(MathUtilsAbsTest, EvaluatesAtCompileTime)
{
    static_assert(Abs(-3) == 3);
    static_assert(Abs(-3.0f) == 3.0f);

    SUCCEED();
}

TEST(MathUtilsEpsilonTest, MatchesNumericLimits)
{
    EXPECT_FLOAT_EQ(std::numeric_limits<float>::epsilon(), Epsilon<float>);
    EXPECT_DOUBLE_EQ(std::numeric_limits<double>::epsilon(), Epsilon<double>);
}

TEST(MathUtilsNearlyZeroTest, AcceptsValuesUpToTheTolerance)
{
    EXPECT_TRUE(NearlyZero(0.0f));
    EXPECT_TRUE(NearlyZero(-0.0f));
    EXPECT_TRUE(NearlyZero(FloatEpsilon));
    EXPECT_TRUE(NearlyZero(-FloatEpsilon));
    EXPECT_FALSE(NearlyZero(FloatEpsilon * 2.0f));
}

TEST(MathUtilsNearlyZeroTest, ToleranceIsAbsolute)
{
    EXPECT_FALSE(NearlyZero(1e-6f));
    EXPECT_TRUE(NearlyZero(1e-6f, 1e-5f));
    EXPECT_TRUE(NearlyZero(-1e-6f, 1e-5f));
    EXPECT_FALSE(NearlyZero(1e-4f, 1e-5f));
}

TEST(MathUtilsNearlyZeroTest, ComparesIntegersExactly)
{
    EXPECT_TRUE(NearlyZero(0));
    EXPECT_FALSE(NearlyZero(1));
    EXPECT_FALSE(NearlyZero(-1));
}

TEST(MathUtilsNearlyEqualTest, AcceptsIdenticalValues)
{
    EXPECT_TRUE(NearlyEqual(1.0f, 1.0f));
    EXPECT_TRUE(NearlyEqual(0.0f, -0.0f));
    EXPECT_TRUE(NearlyEqual(Infinity, Infinity));
    EXPECT_TRUE(NearlyEqual(-Infinity, -Infinity));
}

TEST(MathUtilsNearlyEqualTest, RejectsNotANumber)
{
    EXPECT_FALSE(NearlyEqual(NotANumber, NotANumber));
    EXPECT_FALSE(NearlyEqual(NotANumber, 1.0f));
    EXPECT_FALSE(NearlyEqual(1.0f, NotANumber));
}

TEST(MathUtilsNearlyEqualTest, ScalesToleranceWithMagnitude)
{
    EXPECT_TRUE(NearlyEqual(1000000.0f, 1000001.0f, 1e-5f));
    EXPECT_FALSE(NearlyEqual(1.0f, 2.0f, 1e-5f));
    EXPECT_TRUE(NearlyEqual(1.0f, 1.000001f, 1e-5f));
}

TEST(MathUtilsNearlyEqualTest, FallsBackToAbsoluteToleranceNearZero)
{
    EXPECT_TRUE(NearlyEqual(1e-9f, 2e-9f));
    EXPECT_FALSE(NearlyEqual(0.0f, 1e-3f));
    EXPECT_TRUE(NearlyEqual(0.0f, 1e-3f, 1e-2f));
}

TEST(MathUtilsNearlyEqualTest, IsSymmetric)
{
    EXPECT_EQ(NearlyEqual(3.0f, 3.2f, 0.1f), NearlyEqual(3.2f, 3.0f, 0.1f));
    EXPECT_EQ(NearlyEqual(3.0f, 5.0f, 0.1f), NearlyEqual(5.0f, 3.0f, 0.1f));
}

TEST(MathUtilsNearlyEqualTest, ComparesIntegersExactly)
{
    EXPECT_TRUE(NearlyEqual(5, 5));
    EXPECT_FALSE(NearlyEqual(5, 6));
    EXPECT_FALSE(NearlyEqual(-1, 1));
}

TEST(MathUtilsVectorTest, NearlyEqualComparesEveryComponent)
{
    const Vec3 lhs{1.0f, 2.0f, 3.0f};
    const Vec3 rhs{1.0f, 2.0f, 3.0f};
    const Vec3 other{1.0f, 2.0f, 3.5f};

    EXPECT_TRUE(NearlyEqual(lhs, rhs));
    EXPECT_FALSE(NearlyEqual(lhs, other));
    EXPECT_TRUE(NearlyEqual(lhs, other, 0.2f));
}

TEST(MathUtilsVectorTest, NearlyEqualComparesIntegerVectorsExactly)
{
    const Vec3i lhs{1, 2, 3};
    const Vec3i rhs{1, 2, 3};
    const Vec3i other{1, 2, 4};

    EXPECT_TRUE(NearlyEqual(lhs, rhs));
    EXPECT_FALSE(NearlyEqual(lhs, other));
}

TEST(MathUtilsVectorTest, NearlyZeroChecksEveryComponent)
{
    EXPECT_TRUE(NearlyZero(vec3::Zero));
    EXPECT_FALSE(NearlyZero(Vec3{0.0f, 0.0f, 1e-3f}));
    EXPECT_TRUE(NearlyZero(Vec3{1e-6f, -1e-6f, 0.0f}, 1e-5f));
}

TEST(MathUtilsMatrixTest, NearlyEqualComparesEveryElement)
{
    const Mat2 lhs = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        3.0f, 4.0f
    );

    const Mat2 other = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        3.0f, 4.5f
    );

    EXPECT_TRUE(NearlyEqual(lhs, lhs));
    EXPECT_FALSE(NearlyEqual(lhs, other));
    EXPECT_TRUE(NearlyEqual(lhs, other, 0.2f));
}

TEST(MathUtilsMatrixTest, NearlyEqualComparesIntegerMatricesExactly)
{
    const Matrix<int32, 2, 2> lhs = mat::Make<int32, 2, 2>(
        1, 2,
        3, 4
    );

    const Matrix<int32, 2, 2> other = mat::Make<int32, 2, 2>(
        1, 2,
        3, 5
    );

    EXPECT_TRUE(NearlyEqual(lhs, lhs));
    EXPECT_FALSE(NearlyEqual(lhs, other));
}

TEST(MathUtilsAngleTest, ConvertsDegreesToRadians)
{
    EXPECT_FLOAT_EQ(0.0f, Radians(0.0f));
    EXPECT_NEAR(Pi * 0.5f, Radians(90.0f), 1e-3f);
    EXPECT_NEAR(Pi, Radians(180.0f), 1e-3f);
    EXPECT_NEAR(-Pi, Radians(-180.0f), 1e-3f);
}

TEST(MathUtilsAngleTest, ConvertsRadiansToDegrees)
{
    EXPECT_FLOAT_EQ(0.0f, Degrees(0.0f));
    EXPECT_NEAR(90.0f, Degrees(Pi * 0.5f), 1e-2f);
    EXPECT_NEAR(180.0f, Degrees(Pi), 1e-2f);
}

TEST(MathUtilsAngleTest, RoundTripsWithinConstantPrecision)
{
    EXPECT_TRUE(NearlyEqual(45.0f, Degrees(Radians(45.0f)), 1e-3f));
    EXPECT_TRUE(NearlyEqual(270.0f, Degrees(Radians(270.0f)), 1e-3f));
    EXPECT_TRUE(NearlyEqual(-90.0f, Degrees(Radians(-90.0f)), 1e-3f));
}

TEST(MathUtilsAngleTest, IsLinear)
{
    EXPECT_FLOAT_EQ(Radians(90.0f) * 2.0f, Radians(180.0f));
    EXPECT_FLOAT_EQ(Degrees(1.0f) * 3.0f, Degrees(3.0f));
}

TEST(MathUtilsConstexprTest, PredicatesEvaluateAtCompileTime)
{
    static_assert(NearlyZero(0.0f));
    static_assert(NearlyEqual(1.0f, 1.0f));
    static_assert(NearlyEqual(1, 1));
    static_assert(NearlyEqual(Vec3{1.0f, 2.0f, 3.0f}, Vec3{1.0f, 2.0f, 3.0f}));
    static_assert(NearlyZero(vec3::Zero));
    static_assert(Radians(0.0f) == 0.0f);

    SUCCEED();
}
