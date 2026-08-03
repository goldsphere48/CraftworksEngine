#include <gtest/gtest.h>

#include <math/math_utils.h>
#include <math/vector.h>

#include <type_traits>

using namespace cw;
using namespace cw::math;
using namespace cw::math::vec;

namespace
{
    constexpr float Tolerance = 1e-5f;
}

TEST(VectorStorageTest, MakeStoresComponentsInData)
{
    auto v1 = Make<int32>(7);
    auto v2 = Make<int32>(1, 2);
    auto v3 = Make<int32>(1, 2, 3);
    auto v4 = Make<int32>(1, 2, 3, 4);

    EXPECT_EQ(7, v1.Data[0]);

    EXPECT_EQ(1, v2.Data[0]);
    EXPECT_EQ(2, v2.Data[1]);

    EXPECT_EQ(1, v3.Data[0]);
    EXPECT_EQ(2, v3.Data[1]);
    EXPECT_EQ(3, v3.Data[2]);

    EXPECT_EQ(1, v4.Data[0]);
    EXPECT_EQ(2, v4.Data[1]);
    EXPECT_EQ(3, v4.Data[2]);
    EXPECT_EQ(4, v4.Data[3]);
}

TEST(VectorStorageTest, MakeDeducesDimensionFromArgumentCount)
{
    auto v2 = Make<float>(1.0f, 2.0f);
    auto v3 = Make<float>(1.0f, 2.0f, 3.0f);
    auto v4 = Make<float>(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE((std::is_same_v<decltype(v2), Vec2>));
    EXPECT_TRUE((std::is_same_v<decltype(v3), Vec3>));
    EXPECT_TRUE((std::is_same_v<decltype(v4), Vec4>));
}

TEST(VectorStorageTest, ShorthandAliasesNameExpectedTypes)
{
    EXPECT_TRUE((std::is_same_v<Vec2, Vector<float, 2>>));
    EXPECT_TRUE((std::is_same_v<Vec3, Vector<float, 3>>));
    EXPECT_TRUE((std::is_same_v<Vec4, Vector<float, 4>>));

    EXPECT_TRUE((std::is_same_v<Vec3f, Vector<float, 3>>));
    EXPECT_TRUE((std::is_same_v<Vec3i, Vector<int32, 3>>));
}

TEST(VectorStorageTest, NamedAccessorsAliasData)
{
    auto v = Make<int32>(1, 2, 3, 4);

    EXPECT_EQ(v.Data[0], v.X);
    EXPECT_EQ(v.Data[1], v.Y);
    EXPECT_EQ(v.Data[2], v.Z);
    EXPECT_EQ(v.Data[3], v.W);

    EXPECT_EQ(v.X, v.R);
    EXPECT_EQ(v.Y, v.G);
    EXPECT_EQ(v.Z, v.B);
    EXPECT_EQ(v.W, v.A);
}

TEST(VectorStorageTest, WritingThroughNamedAccessorUpdatesData)
{
    auto v = Make<int32>(1, 2, 3);

    v.Y = 42;

    EXPECT_EQ(42, v.Data[1]);
    EXPECT_EQ(42, v.G);
}

TEST(VectorStorageTest, HasNoPaddingAroundComponents)
{
    EXPECT_EQ(sizeof(float) * 2, sizeof(Vec2));
    EXPECT_EQ(sizeof(float) * 3, sizeof(Vec3));
    EXPECT_EQ(sizeof(float) * 4, sizeof(Vec4));
    EXPECT_EQ(sizeof(float) * 7, sizeof(Vector<float, 7>));
}

TEST(VectorFactoryTest, FillRepeatsTheValue)
{
    const auto v = Fill<float, 5>(2.5f);

    for (usize i = 0; i < 5; ++i)
    {
        EXPECT_FLOAT_EQ(2.5f, v.Data[i]);
    }
}

TEST(VectorFactoryTest, AxisSetsASingleComponent)
{
    const auto v = Axis<float, 4>(2, 3.0f);

    EXPECT_FLOAT_EQ(0.0f, v.X);
    EXPECT_FLOAT_EQ(0.0f, v.Y);
    EXPECT_FLOAT_EQ(3.0f, v.Z);
    EXPECT_FLOAT_EQ(0.0f, v.W);
}

TEST(VectorFactoryTest, ZeroAndOneFillEveryComponent)
{
    for (usize i = 0; i < 4; ++i)
    {
        EXPECT_FLOAT_EQ(0.0f, (Zero<float, 4>).Data[i]);
        EXPECT_FLOAT_EQ(1.0f, (One<float, 4>).Data[i]);
    }
}

TEST(VectorFactoryTest, UnitVectorsPointAlongTheirAxis)
{
    EXPECT_TRUE(NearlyEqual(Vec4{1.0f, 0.0f, 0.0f, 0.0f}, (UnitX<float, 4>)));
    EXPECT_TRUE(NearlyEqual(Vec4{0.0f, 1.0f, 0.0f, 0.0f}, (UnitY<float, 4>)));
    EXPECT_TRUE(NearlyEqual(Vec4{0.0f, 0.0f, 1.0f, 0.0f}, (UnitZ<float, 4>)));
    EXPECT_TRUE(NearlyEqual(Vec4{0.0f, 0.0f, 0.0f, 1.0f}, (UnitW<float, 4>)));
}

TEST(VectorFactoryTest, ShorthandConstantsMatchTheirTemplateCounterparts)
{
    EXPECT_TRUE(NearlyEqual(vec2::Zero, (Zero<float, 2>)));
    EXPECT_TRUE(NearlyEqual(vec2::One, (One<float, 2>)));
    EXPECT_TRUE(NearlyEqual(vec2::UnitY, (UnitY<float, 2>)));

    EXPECT_TRUE(NearlyEqual(vec3::Zero, (Zero<float, 3>)));
    EXPECT_TRUE(NearlyEqual(vec3::UnitZ, (UnitZ<float, 3>)));

    EXPECT_TRUE(NearlyEqual(vec4::One, (One<float, 4>)));
    EXPECT_TRUE(NearlyEqual(vec4::UnitW, (UnitW<float, 4>)));
}

TEST(VectorArithmeticTest, AddsAndSubtractsComponentWise)
{
    const Vec3 lhs{1.0f, 2.0f, 3.0f};
    const Vec3 rhs{4.0f, -5.0f, 6.0f};

    EXPECT_TRUE(NearlyEqual(Vec3{5.0f, -3.0f, 9.0f}, lhs + rhs));
    EXPECT_TRUE(NearlyEqual(Vec3{-3.0f, 7.0f, -3.0f}, lhs - rhs));
}

TEST(VectorArithmeticTest, ScalesAndDividesEveryComponent)
{
    const Vec3 v{1.0f, -2.0f, 3.0f};

    EXPECT_TRUE(NearlyEqual(Vec3{2.0f, -4.0f, 6.0f}, v * 2.0f));
    EXPECT_TRUE(NearlyEqual(Vec3{0.5f, -1.0f, 1.5f}, v / 2.0f));
}

TEST(VectorArithmeticTest, AcceptsSmallNonZeroDivisor)
{
    Vec3 v{1e-4f, 2e-4f, 3e-4f};

    v /= 1e-4f;

    EXPECT_TRUE(NearlyEqual(Vec3{1.0f, 2.0f, 3.0f}, v, Tolerance));
}

TEST(VectorArithmeticTest, BinaryOperatorsLeaveOperandsUntouched)
{
    const Vec3 lhs{1.0f, 2.0f, 3.0f};
    const Vec3 rhs{4.0f, 5.0f, 6.0f};

    const Vec3 sum = lhs + rhs;
    const Vec3 scaled = lhs * 3.0f;

    EXPECT_TRUE(NearlyEqual(Vec3{1.0f, 2.0f, 3.0f}, lhs));
    EXPECT_TRUE(NearlyEqual(Vec3{4.0f, 5.0f, 6.0f}, rhs));
    EXPECT_TRUE(NearlyEqual(Vec3{5.0f, 7.0f, 9.0f}, sum));
    EXPECT_TRUE(NearlyEqual(Vec3{3.0f, 6.0f, 9.0f}, scaled));
}

TEST(VectorArithmeticTest, CompoundAssignmentMutatesInPlace)
{
    Vec3 v{1.0f, 2.0f, 3.0f};

    v += Vec3{1.0f, 1.0f, 1.0f};
    EXPECT_TRUE(NearlyEqual(Vec3{2.0f, 3.0f, 4.0f}, v));

    v -= Vec3{2.0f, 2.0f, 2.0f};
    EXPECT_TRUE(NearlyEqual(Vec3{0.0f, 1.0f, 2.0f}, v));

    v *= 4.0f;
    EXPECT_TRUE(NearlyEqual(Vec3{0.0f, 4.0f, 8.0f}, v));

    v /= 2.0f;
    EXPECT_TRUE(NearlyEqual(Vec3{0.0f, 2.0f, 4.0f}, v));
}

TEST(VectorArithmeticTest, CompoundAssignmentReturnsTheSameObject)
{
    Vec3 v{1.0f, 2.0f, 3.0f};

    EXPECT_EQ(&v, &(v += vec3::One));
    EXPECT_EQ(&v, &(v -= vec3::One));
    EXPECT_EQ(&v, &(v *= 2.0f));
    EXPECT_EQ(&v, &(v /= 2.0f));
}

TEST(VectorArithmeticTest, WorksOnIntegerVectors)
{
    const Vec3i lhs{1, 2, 3};
    const Vec3i rhs{4, 5, 6};

    EXPECT_TRUE(NearlyEqual(Vec3i{5, 7, 9}, lhs + rhs));
    EXPECT_TRUE(NearlyEqual(Vec3i{-3, -3, -3}, lhs - rhs));
    EXPECT_TRUE(NearlyEqual(Vec3i{2, 4, 6}, lhs * 2));
    EXPECT_TRUE(NearlyEqual(Vec3i{0, 1, 1}, lhs / 2));
}

TEST(VectorLengthTest, MeasuresPythagoreanTriples)
{
    EXPECT_FLOAT_EQ(5.0f, Length(Vec2{3.0f, 4.0f}));
    EXPECT_FLOAT_EQ(13.0f, Length(Vec3{3.0f, 4.0f, 12.0f}));
    EXPECT_FLOAT_EQ(0.0f, Length(vec3::Zero));
}

TEST(VectorLengthTest, IgnoresComponentSigns)
{
    EXPECT_FLOAT_EQ(Length(Vec3{3.0f, 4.0f, 12.0f}), Length(Vec3{-3.0f, -4.0f, -12.0f}));
}

TEST(VectorLengthTest, ScalesLinearly)
{
    const Vec3 v{1.0f, -2.0f, 2.0f};

    EXPECT_FLOAT_EQ(3.0f, Length(v));
    EXPECT_FLOAT_EQ(9.0f, Length(v * 3.0f));
}

TEST(VectorLengthTest, SquaredMatchesLengthTimesLength)
{
    const Vec4 v{1.0f, -2.0f, 3.0f, -4.0f};

    EXPECT_FLOAT_EQ(30.0f, LengthSquared(v));
    EXPECT_NEAR(Length(v) * Length(v), LengthSquared(v), 1e-4f);
}

TEST(VectorLengthTest, WidensIntegerLengthToDouble)
{
    const Vec3i v{1, 2, 2};

    EXPECT_TRUE((std::is_same_v<decltype(Length(v)), double>));
    EXPECT_DOUBLE_EQ(3.0, Length(v));

    EXPECT_TRUE((std::is_same_v<decltype(Length(Vec3{})), float>));
}

TEST(VectorLengthTest, SquaredKeepsTheElementType)
{
    const Vec3i v{1, 2, 2};

    EXPECT_TRUE((std::is_same_v<decltype(LengthSquared(v)), int32>));
    EXPECT_EQ(9, LengthSquared(v));
}

TEST(VectorLengthTest, IntegerLengthDoesNotTruncateBeforeTheRoot)
{
    const Vec2i v{40000, 30000};

    EXPECT_DOUBLE_EQ(50000.0, Length(v));
}

TEST(VectorDistanceTest, MeasuresTheGapBetweenPoints)
{
    const Vec3 lhs{1.0f, 2.0f, 3.0f};
    const Vec3 rhs{4.0f, 6.0f, 3.0f};

    EXPECT_FLOAT_EQ(5.0f, Distance(lhs, rhs));
    EXPECT_FLOAT_EQ(25.0f, DistanceSquared(lhs, rhs));
}

TEST(VectorDistanceTest, IsSymmetricAndZeroForEqualPoints)
{
    const Vec3 lhs{1.0f, 2.0f, 3.0f};
    const Vec3 rhs{-4.0f, 6.0f, 0.5f};

    EXPECT_FLOAT_EQ(Distance(lhs, rhs), Distance(rhs, lhs));
    EXPECT_FLOAT_EQ(0.0f, Distance(lhs, lhs));
}

TEST(VectorDistanceTest, MatchesLengthOfTheDifference)
{
    const Vec3 lhs{1.0f, 2.0f, 3.0f};
    const Vec3 rhs{-4.0f, 6.0f, 0.5f};

    EXPECT_FLOAT_EQ(Length(rhs - lhs), Distance(lhs, rhs));
}

TEST(VectorDistanceTest, WidensIntegerDistanceToDouble)
{
    const Vec3i lhs{0, 0, 0};
    const Vec3i rhs{3, 4, 0};

    EXPECT_TRUE((std::is_same_v<decltype(Distance(lhs, rhs)), double>));
    EXPECT_DOUBLE_EQ(5.0, Distance(lhs, rhs));
    EXPECT_EQ(25, DistanceSquared(lhs, rhs));
}

TEST(VectorDistanceTest, IntegerDistanceDoesNotWrapAroundOnLargeGaps)
{
    const Vec2i lhs{-40000, 0};
    const Vec2i rhs{0, 30000};

    EXPECT_DOUBLE_EQ(50000.0, Distance(lhs, rhs));
}

TEST(VectorNormalizeTest, ProducesUnitLength)
{
    const Vec3 v{3.0f, -4.0f, 12.0f};

    EXPECT_NEAR(1.0f, Length(Normalize(v)), Tolerance);
}

TEST(VectorNormalizeTest, KeepsTheDirection)
{
    const Vec3 v{0.0f, 3.0f, 4.0f};

    EXPECT_TRUE(NearlyEqual(Vec3{0.0f, 0.6f, 0.8f}, Normalize(v), Tolerance));
}

TEST(VectorNormalizeTest, IgnoresTheMagnitudeOfTheInput)
{
    const Vec3 v{1.0f, 2.0f, 3.0f};

    EXPECT_TRUE(NearlyEqual(Normalize(v), Normalize(v * 100.0f), Tolerance));
}

TEST(VectorNormalizeTest, LeavesUnitVectorsUnchanged)
{
    EXPECT_TRUE(NearlyEqual(vec3::UnitY, Normalize(vec3::UnitY), Tolerance));
}

TEST(VectorNormalizeTest, HandlesVerySmallVectors)
{
    const Vec3 v{1e-8f, 0.0f, 0.0f};

    EXPECT_TRUE(NearlyEqual(vec3::UnitX, Normalize(v), Tolerance));
}

TEST(VectorNormalizeTest, WidensIntegerInputToFloat)
{
    const Vec3i v{0, 3, 4};

    const auto normalized = Normalize(v);

    EXPECT_TRUE((std::is_same_v<decltype(normalized), const Vec3>));
    EXPECT_TRUE(NearlyEqual(Vec3{0.0f, 0.6f, 0.8f}, normalized, Tolerance));
}

TEST(VectorDotTest, SelfDotEqualsLengthSquared)
{
    const Vec3 v{1.0f, -2.0f, 3.0f};

    EXPECT_FLOAT_EQ(LengthSquared(v), Dot(v, v));
}

TEST(VectorDotTest, IsCommutative)
{
    const Vec3 lhs{1.0f, -2.0f, 3.0f};
    const Vec3 rhs{4.0f, 5.0f, -6.0f};

    EXPECT_FLOAT_EQ(Dot(lhs, rhs), Dot(rhs, lhs));
}

TEST(VectorDotTest, VanishesForOrthogonalVectors)
{
    EXPECT_FLOAT_EQ(0.0f, Dot(vec3::UnitX, vec3::UnitY));
    EXPECT_FLOAT_EQ(0.0f, Dot(vec3::UnitY, vec3::UnitZ));
    EXPECT_FLOAT_EQ(0.0f, Dot(Vec2{1.0f, 2.0f}, Vec2{-2.0f, 1.0f}));
}

TEST(VectorDotTest, SignFollowsTheAngle)
{
    const Vec3 v{1.0f, 1.0f, 0.0f};

    EXPECT_GT(Dot(v, vec3::UnitX), 0.0f);
    EXPECT_LT(Dot(v, vec3::UnitX * -1.0f), 0.0f);
}

TEST(VectorDotTest, IsLinearInBothArguments)
{
    const Vec3 a{1.0f, 2.0f, 3.0f};
    const Vec3 b{-1.0f, 0.5f, 2.0f};
    const Vec3 c{4.0f, -2.0f, 1.0f};

    EXPECT_NEAR(Dot(a, b) + Dot(a, c), Dot(a, b + c), 1e-4f);
    EXPECT_NEAR(Dot(a, b) * 3.0f, Dot(a * 3.0f, b), 1e-4f);
}

TEST(VectorDotTest, KeepsTheElementType)
{
    const Vec3i v{1, 2, 3};

    EXPECT_TRUE((std::is_same_v<decltype(Dot(v, v)), int32>));
    EXPECT_EQ(14, Dot(v, v));
}

TEST(VectorCrossTest, FollowsRightHandRuleForBasisVectors)
{
    EXPECT_TRUE(NearlyEqual(vec3::UnitZ, Cross(vec3::UnitX, vec3::UnitY)));
    EXPECT_TRUE(NearlyEqual(vec3::UnitX, Cross(vec3::UnitY, vec3::UnitZ)));
    EXPECT_TRUE(NearlyEqual(vec3::UnitY, Cross(vec3::UnitZ, vec3::UnitX)));
}

TEST(VectorCrossTest, IsAntiCommutative)
{
    const Vec3 lhs{1.0f, 2.0f, 3.0f};
    const Vec3 rhs{-4.0f, 5.0f, 6.0f};

    EXPECT_TRUE(NearlyEqual(Cross(lhs, rhs), Cross(rhs, lhs) * -1.0f));
}

TEST(VectorCrossTest, IsOrthogonalToBothOperands)
{
    const Vec3 lhs{1.0f, 2.0f, 3.0f};
    const Vec3 rhs{-4.0f, 5.0f, 6.0f};

    const Vec3 product = Cross(lhs, rhs);

    EXPECT_NEAR(0.0f, Dot(product, lhs), 1e-4f);
    EXPECT_NEAR(0.0f, Dot(product, rhs), 1e-4f);
}

TEST(VectorCrossTest, VanishesForParallelVectors)
{
    const Vec3 v{1.0f, 2.0f, 3.0f};

    EXPECT_TRUE(NearlyZero(Cross(v, v)));
    EXPECT_TRUE(NearlyZero(Cross(v, v * 2.5f), 1e-5f));
}

TEST(VectorCrossTest, TwoDimensionalVersionReturnsSignedArea)
{
    EXPECT_FLOAT_EQ(1.0f, Cross(vec2::UnitX, vec2::UnitY));
    EXPECT_FLOAT_EQ(-1.0f, Cross(vec2::UnitY, vec2::UnitX));
    EXPECT_FLOAT_EQ(0.0f, Cross(Vec2{2.0f, 4.0f}, Vec2{1.0f, 2.0f}));
    EXPECT_FLOAT_EQ(-2.0f, Cross(Vec2{2.0f, 0.0f}, Vec2{3.0f, -1.0f}));
}

TEST(VectorCrossTest, MatchesTheZComponentOfTheThreeDimensionalVersion)
{
    const Vec2 lhs{1.0f, 2.0f};
    const Vec2 rhs{-3.0f, 4.0f};

    const Vec3 lifted = Cross(Vec3{lhs.X, lhs.Y, 0.0f}, Vec3{rhs.X, rhs.Y, 0.0f});

    EXPECT_FLOAT_EQ(lifted.Z, Cross(lhs, rhs));
}

TEST(VectorCrossTest, IntegerCrossDoesNotOverflowInIntermediateProducts)
{
    const Vector<int32, 3> lhs{100000, 0, 0};
    const Vector<int32, 3> rhs{0, 20000, 0};

    const auto product = Cross(lhs, rhs);

    EXPECT_EQ(0, product.X);
    EXPECT_EQ(0, product.Y);
    EXPECT_EQ(2000000000, product.Z);
}

TEST(VectorPerpTest, TurnsByAQuarterTurn)
{
    const Vec2 v{2.0f, 1.0f};

    EXPECT_TRUE(NearlyEqual(Vec2{-1.0f, 2.0f}, PerpCCW(v)));
    EXPECT_TRUE(NearlyEqual(Vec2{1.0f, -2.0f}, PerpCW(v)));
}

TEST(VectorPerpTest, IsOrthogonalAndLengthPreserving)
{
    const Vec2 v{2.0f, 1.0f};

    EXPECT_FLOAT_EQ(0.0f, Dot(v, PerpCCW(v)));
    EXPECT_FLOAT_EQ(0.0f, Dot(v, PerpCW(v)));
    EXPECT_FLOAT_EQ(Length(v), Length(PerpCCW(v)));
}

TEST(VectorPerpTest, OppositeDirectionsAreNegatives)
{
    const Vec2 v{2.0f, 1.0f};

    EXPECT_TRUE(NearlyEqual(PerpCCW(v), PerpCW(v) * -1.0f));
    EXPECT_GT(Cross(v, PerpCCW(v)), 0.0f);
    EXPECT_LT(Cross(v, PerpCW(v)), 0.0f);
}

TEST(VectorConstexprTest, ExpressionsEvaluateAtCompileTime)
{
    constexpr Vec3 sum = Vec3{1.0f, 2.0f, 3.0f} + Vec3{1.0f, 1.0f, 1.0f};
    static_assert(sum.Data[1] == 3.0f);

    constexpr Vec3 scaled = Vec3{1.0f, 2.0f, 3.0f} * 2.0f;
    static_assert(scaled.Data[2] == 6.0f);

    constexpr Vec3 difference = Vec3{1.0f, 2.0f, 3.0f} - Vec3{1.0f, 1.0f, 1.0f};
    static_assert(difference.Data[0] == 0.0f);

    constexpr float dot = Dot(Vec3{1.0f, 2.0f, 3.0f}, Vec3{4.0f, 5.0f, 6.0f});
    static_assert(dot == 32.0f);

    constexpr float lengthSquared = LengthSquared(Vec3{1.0f, 2.0f, 2.0f});
    static_assert(lengthSquared == 9.0f);

    static_assert((Zero<float, 3>).Data[0] == 0.0f);
    static_assert((UnitZ<float, 3>).Data[2] == 1.0f);

    SUCCEED();
}
