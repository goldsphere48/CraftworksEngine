#include <gtest/gtest.h>

#include <math/vector.h>
#include <world/coordinate_system.h>

using namespace cw;
using namespace cw::math;
using namespace cw::math::vec;

TEST(VectorStorageTest, MakeStoresComponentsInData)
{
    auto v1 = Make<int>(7);
    auto v2 = Make<int>(1, 2);
    auto v3 = Make<int>(1, 2, 3);
    auto v4 = Make<int>(1, 2, 3, 4);

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
    EXPECT_TRUE((std::is_same_v<Vec3, Vector<float, 3>>));
    EXPECT_TRUE((std::is_same_v<Vec3f, Vector<float, 3>>));
    EXPECT_TRUE((std::is_same_v<Vec3i, Vector<int32, 3>>));
}

TEST(VectorStorageTest, NamedAccessorsAliasData)
{
    auto v = Make<int>(1, 2, 3, 4);

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
    auto v = Make<int>(1, 2, 3);

    v.Y = 42;

    EXPECT_EQ(42, v.Data[1]);
    EXPECT_EQ(42, v.G);
}

TEST(VectorFactoryTest, ZeroHasEveryComponentZero)
{
    auto v = Zero<float, 4>;

    for (usize i = 0; i < 4; ++i)
    {
        EXPECT_FLOAT_EQ(0.0f, v.Data[i]);
    }
}

TEST(VectorFactoryTest, OneHasEveryComponentOne)
{
    auto v = One<int, 3>;

    for (usize i = 0; i < 3; ++i)
    {
        EXPECT_EQ(1, v.Data[i]);
    }
}

TEST(VectorFactoryTest, FillSetsEveryComponent)
{
    auto v = Fill<int, 4>(9);

    for (usize i = 0; i < 4; ++i)
    {
        EXPECT_EQ(9, v.Data[i]);
    }
}

TEST(VectorFactoryTest, AxisSetsOnlyRequestedComponent)
{
    auto v = Axis<float, 4>(2, 5.0f);

    EXPECT_FLOAT_EQ(0.0f, v.X);
    EXPECT_FLOAT_EQ(0.0f, v.Y);
    EXPECT_FLOAT_EQ(5.0f, v.Z);
    EXPECT_FLOAT_EQ(0.0f, v.W);
}

TEST(VectorUnitTest, UnitVectorsSelectTheirOwnAxis)
{
    auto x = UnitX<float, 3>;
    auto y = UnitY<float, 3>;
    auto z = UnitZ<float, 3>;

    EXPECT_FLOAT_EQ(1.0f, x.X);
    EXPECT_FLOAT_EQ(0.0f, x.Y);
    EXPECT_FLOAT_EQ(0.0f, x.Z);

    EXPECT_FLOAT_EQ(0.0f, y.X);
    EXPECT_FLOAT_EQ(1.0f, y.Y);
    EXPECT_FLOAT_EQ(0.0f, y.Z);

    EXPECT_FLOAT_EQ(0.0f, z.X);
    EXPECT_FLOAT_EQ(0.0f, z.Y);
    EXPECT_FLOAT_EQ(1.0f, z.Z);
}

TEST(VectorUnitTest, UnitVectorsHaveUnitLength)
{
    EXPECT_FLOAT_EQ(1.0f, Length(UnitX<float, 3>));
    EXPECT_FLOAT_EQ(1.0f, Length(UnitY<float, 3>));
    EXPECT_FLOAT_EQ(1.0f, Length(UnitZ<float, 3>));
}

TEST(VectorUnitTest, UnitVectorsAreMutuallyOrthogonal)
{
    EXPECT_FLOAT_EQ(0.0f, Dot(UnitX<float, 3>, UnitY<float, 3>));
    EXPECT_FLOAT_EQ(0.0f, Dot(UnitX<float, 3>, UnitZ<float, 3>));
    EXPECT_FLOAT_EQ(0.0f, Dot(UnitY<float, 3>, UnitZ<float, 3>));
}

TEST(VectorUnitTest, UnitVectorsFormRightHandedBasis)
{
    auto z = Cross(UnitX<float, 3>, UnitY<float, 3>);

    EXPECT_FLOAT_EQ(0.0f, z.X);
    EXPECT_FLOAT_EQ(0.0f, z.Y);
    EXPECT_FLOAT_EQ(1.0f, z.Z);
}

TEST(WorldDirectionTest, RightAndLeftLieOnX)
{
    EXPECT_FLOAT_EQ(1.0f, world::Right.X);
    EXPECT_FLOAT_EQ(0.0f, world::Right.Y);
    EXPECT_FLOAT_EQ(0.0f, world::Right.Z);

    EXPECT_FLOAT_EQ(-1.0f, world::Left.X);
    EXPECT_FLOAT_EQ(0.0f, world::Left.Y);
    EXPECT_FLOAT_EQ(0.0f, world::Left.Z);
}

TEST(WorldDirectionTest, UpAndDownLieOnY)
{
    EXPECT_FLOAT_EQ(0.0f, world::Up.X);
    EXPECT_FLOAT_EQ(1.0f, world::Up.Y);
    EXPECT_FLOAT_EQ(0.0f, world::Up.Z);

    EXPECT_FLOAT_EQ(0.0f, world::Down.X);
    EXPECT_FLOAT_EQ(-1.0f, world::Down.Y);
    EXPECT_FLOAT_EQ(0.0f, world::Down.Z);
}

TEST(WorldDirectionTest, ForwardIsNegativeZAndBackIsPositiveZ)
{
    EXPECT_FLOAT_EQ(0.0f, world::Forward.X);
    EXPECT_FLOAT_EQ(0.0f, world::Forward.Y);
    EXPECT_FLOAT_EQ(-1.0f, world::Forward.Z);

    EXPECT_FLOAT_EQ(0.0f, world::Back.X);
    EXPECT_FLOAT_EQ(0.0f, world::Back.Y);
    EXPECT_FLOAT_EQ(1.0f, world::Back.Z);
}

TEST(WorldDirectionTest, OppositesCancelOut)
{
    EXPECT_FLOAT_EQ(0.0f, Length(world::Right + world::Left));
    EXPECT_FLOAT_EQ(0.0f, Length(world::Up + world::Down));
    EXPECT_FLOAT_EQ(0.0f, Length(world::Forward + world::Back));
}

TEST(WorldDirectionTest, AreUnitLength)
{
    EXPECT_FLOAT_EQ(1.0f, Length(world::Right));
    EXPECT_FLOAT_EQ(1.0f, Length(world::Left));
    EXPECT_FLOAT_EQ(1.0f, Length(world::Up));
    EXPECT_FLOAT_EQ(1.0f, Length(world::Down));
    EXPECT_FLOAT_EQ(1.0f, Length(world::Forward));
    EXPECT_FLOAT_EQ(1.0f, Length(world::Back));
}

TEST(WorldDirectionTest, AreMutuallyOrthogonal)
{
    EXPECT_FLOAT_EQ(0.0f, Dot(world::Right, world::Up));
    EXPECT_FLOAT_EQ(0.0f, Dot(world::Right, world::Forward));
    EXPECT_FLOAT_EQ(0.0f, Dot(world::Up, world::Forward));
}

TEST(WorldDirectionTest, BasisIsRightHanded)
{
    auto back = Cross(world::Right, world::Up);

    EXPECT_FLOAT_EQ(world::Back.X, back.X);
    EXPECT_FLOAT_EQ(world::Back.Y, back.Y);
    EXPECT_FLOAT_EQ(world::Back.Z, back.Z);
}

TEST(WorldDirectionTest, MatchesMathUnitVectorsOnPositiveAxes)
{
    EXPECT_FLOAT_EQ(0.0f, Distance(world::Right, UnitX<float, 3>));
    EXPECT_FLOAT_EQ(0.0f, Distance(world::Up, UnitY<float, 3>));
    EXPECT_FLOAT_EQ(0.0f, Distance(world::Back, UnitZ<float, 3>));
}

TEST(VectorArithmeticTest, AddsComponentWise)
{
    auto c = Make<int>(1, 2, 3) + Make<int>(4, 5, 6);

    EXPECT_EQ(5, c.X);
    EXPECT_EQ(7, c.Y);
    EXPECT_EQ(9, c.Z);
}

TEST(VectorArithmeticTest, SubtractsComponentWise)
{
    auto c = Make<int>(1, 2, 3) - Make<int>(4, 5, 6);

    EXPECT_EQ(-3, c.X);
    EXPECT_EQ(-3, c.Y);
    EXPECT_EQ(-3, c.Z);
}

TEST(VectorArithmeticTest, ScalesByScalar)
{
    auto c = Make<int>(1, 2, 3) * 2;

    EXPECT_EQ(2, c.X);
    EXPECT_EQ(4, c.Y);
    EXPECT_EQ(6, c.Z);
}

TEST(VectorArithmeticTest, DividesByScalar)
{
    auto c = Make<float>(2.0f, 4.0f, 6.0f) / 2.0f;

    EXPECT_FLOAT_EQ(1.0f, c.X);
    EXPECT_FLOAT_EQ(2.0f, c.Y);
    EXPECT_FLOAT_EQ(3.0f, c.Z);
}

TEST(VectorArithmeticTest, AcceptsSmallNonZeroDivisor)
{
    auto c = Make<float>(1.0f, 2.0f, 3.0f) / 1e-8f;

    EXPECT_FLOAT_EQ(1e8f, c.X);
    EXPECT_FLOAT_EQ(3e8f, c.Z);
}

TEST(VectorNormalizeTest, AcceptsVeryShortVector)
{
    auto v = Normalize(Make<float>(0.0f, 1e-20f));

    EXPECT_FLOAT_EQ(0.0f, v.X);
    EXPECT_FLOAT_EQ(1.0f, v.Y);
}

TEST(VectorArithmeticTest, BinaryOperatorsLeaveOperandsUntouched)
{
    auto a = Make<int>(1, 2, 3);
    auto b = Make<int>(4, 5, 6);

    auto c = a + b;

    EXPECT_EQ(1, a.X);
    EXPECT_EQ(4, b.X);
    EXPECT_EQ(5, c.X);
}

TEST(VectorArithmeticTest, CompoundAssignmentMutatesInPlace)
{
    auto a = Make<int>(1, 2, 3);

    a += Make<int>(4, 5, 6);
    EXPECT_EQ(5, a.X);

    a -= Make<int>(4, 5, 6);
    EXPECT_EQ(1, a.X);

    a *= 3;
    EXPECT_EQ(3, a.X);

    a /= 3;
    EXPECT_EQ(1, a.X);
}

TEST(VectorArithmeticTest, OperationsResolveThroughAdlWithoutUsingDirective)
{
    cw::math::Vec3i a = cw::math::vec::Make<cw::int32>(1, 2, 3);
    cw::math::Vec3i b = cw::math::vec::Make<cw::int32>(4, 5, 6);

    cw::math::Vec3i c = a + b;

    EXPECT_EQ(5, c.X);
    EXPECT_EQ(32, Dot(a, b));
}

TEST(VectorLengthTest, IntegerLengthPromotesToDouble)
{
    auto v = Make<int>(0, 5);

    EXPECT_TRUE((std::is_same_v<decltype(Length(v)), double>));
    EXPECT_DOUBLE_EQ(5.0, Length(v));
}

TEST(VectorLengthTest, FloatLengthStaysFloat)
{
    auto v = Make<float>(3.0f, 4.0f);

    EXPECT_TRUE((std::is_same_v<decltype(Length(v)), float>));
    EXPECT_FLOAT_EQ(5.0f, Length(v));
}

TEST(VectorLengthTest, HandlesThreeAndFourDimensions)
{
    EXPECT_DOUBLE_EQ(7.0, Length(Make<int>(2, 3, 6)));
    EXPECT_DOUBLE_EQ(5.0, Length(Make<int>(1, 2, 2, 4)));
}

TEST(VectorLengthTest, ZeroVectorHasZeroLength)
{
    EXPECT_FLOAT_EQ(0.0f, Length(Zero<float, 3>));
}

TEST(VectorLengthTest, SquaredMatchesLengthTimesLength)
{
    auto v = Make<float>(3.0f, 4.0f);

    EXPECT_FLOAT_EQ(Length(v) * Length(v), LengthSquared(v));
}

TEST(VectorLengthTest, SquaredWidensIntegerAccumulator)
{
    auto v = Fill<int32, 2>(100000);

    EXPECT_TRUE((std::is_same_v<decltype(LengthSquared(v)), int64>));
    EXPECT_EQ(20000000000LL, LengthSquared(v));
}

TEST(VectorDotTest, ComputesSumOfProducts)
{
    EXPECT_EQ(32, Dot(Make<int>(1, 2, 3), Make<int>(4, 5, 6)));
}

TEST(VectorDotTest, OrthogonalVectorsGiveZero)
{
    EXPECT_EQ(0, Dot(Make<int>(1, 0, 0), Make<int>(0, 1, 0)));
}

TEST(VectorDotTest, IsCommutative)
{
    auto a = Make<int>(1, 2, 3);
    auto b = Make<int>(4, 5, 6);

    EXPECT_EQ(Dot(a, b), Dot(b, a));
}

TEST(VectorDotTest, SelfDotEqualsLengthSquared)
{
    auto v = Make<int>(1, 2, 3);

    EXPECT_EQ(LengthSquared(v), Dot(v, v));
}

TEST(VectorDotTest, WidensIntegerAccumulator)
{
    auto v = Fill<int32, 2>(100000);

    EXPECT_TRUE((std::is_same_v<decltype(Dot(v, v)), int64>));
    EXPECT_EQ(20000000000LL, Dot(v, v));
}

TEST(VectorCrossTest, TwoDimensionalReturnsScalar)
{
    auto a = Make<int>(1, 0);
    auto b = Make<int>(0, 1);

    EXPECT_TRUE((std::is_same_v<decltype(Cross(a, b)), int64>));
    EXPECT_EQ(1, Cross(a, b));
    EXPECT_EQ(-1, Cross(b, a));
}

TEST(VectorCrossTest, TwoDimensionalParallelGivesZero)
{
    EXPECT_EQ(0, Cross(Make<int>(2, 4), Make<int>(1, 2)));
}

TEST(VectorCrossTest, ThreeDimensionalFollowsRightHandRule)
{
    auto z = Cross(Make<int>(1, 0, 0), Make<int>(0, 1, 0));

    EXPECT_EQ(0, z.X);
    EXPECT_EQ(0, z.Y);
    EXPECT_EQ(1, z.Z);
}

TEST(VectorCrossTest, ThreeDimensionalComputesKnownValue)
{
    auto c = Cross(Make<int>(1, 2, 3), Make<int>(4, 5, 6));

    EXPECT_EQ(-3, c.X);
    EXPECT_EQ(6, c.Y);
    EXPECT_EQ(-3, c.Z);
}

TEST(VectorCrossTest, ThreeDimensionalIsAnticommutative)
{
    auto ab = Cross(Make<int>(1, 2, 3), Make<int>(4, 5, 6));
    auto ba = Cross(Make<int>(4, 5, 6), Make<int>(1, 2, 3));

    EXPECT_EQ(-ab.X, ba.X);
    EXPECT_EQ(-ab.Y, ba.Y);
    EXPECT_EQ(-ab.Z, ba.Z);
}

TEST(VectorCrossTest, ThreeDimensionalResultIsOrthogonalToBothOperands)
{
    auto a = Make<int64>(1LL, 2LL, 3LL);
    auto b = Make<int64>(4LL, 5LL, 6LL);

    auto c = Cross(a, b);

    EXPECT_EQ(0, Dot(a, c));
    EXPECT_EQ(0, Dot(b, c));
}

TEST(VectorCrossTest, ThreeDimensionalParallelGivesZeroVector)
{
    auto c = Cross(Make<int>(1, 2, 3), Make<int>(2, 4, 6));

    EXPECT_EQ(0, c.X);
    EXPECT_EQ(0, c.Y);
    EXPECT_EQ(0, c.Z);
}

TEST(VectorDistanceTest, MeasuresBetweenIntegerPoints)
{
    auto a = Make<int>(1, 2);
    auto b = Make<int>(4, 6);

    EXPECT_DOUBLE_EQ(5.0, Distance(a, b));
    EXPECT_EQ(25, DistanceSquared(a, b));
}

TEST(VectorDistanceTest, MeasuresBetweenFloatPoints)
{
    auto a = Make<float>(1.0f, 2.0f);
    auto b = Make<float>(4.0f, 6.0f);

    EXPECT_FLOAT_EQ(5.0f, Distance(a, b));
    EXPECT_FLOAT_EQ(25.0f, DistanceSquared(a, b));
}

TEST(VectorDistanceTest, IsSymmetric)
{
    auto a = Make<int>(1, 2);
    auto b = Make<int>(4, 6);

    EXPECT_DOUBLE_EQ(Distance(a, b), Distance(b, a));
    EXPECT_EQ(DistanceSquared(a, b), DistanceSquared(b, a));
}

TEST(VectorDistanceTest, IsZeroForIdenticalPoints)
{
    auto v = Make<float>(3.0f, 4.0f);

    EXPECT_FLOAT_EQ(0.0f, Distance(v, v));
}

TEST(VectorDistanceTest, IntegerOverloadAvoidsIntermediateOverflow)
{
    auto a = Make<int32>(-2000000000, 0);
    auto b = Make<int32>(2000000000, 0);

    EXPECT_DOUBLE_EQ(4000000000.0, Distance(a, b));
}

TEST(VectorDistanceTest, SquaredWidensIntegerAccumulator)
{
    auto a = Zero<int32, 2>;
    auto b = Fill<int32, 2>(100000);

    EXPECT_EQ(20000000000LL, DistanceSquared(a, b));
}

TEST(VectorNormalizeTest, ProducesUnitLength)
{
    EXPECT_FLOAT_EQ(1.0f, Length(Normalize(Make<float>(3.0f, 4.0f))));
}

TEST(VectorNormalizeTest, PreservesDirection)
{
    auto v = Normalize(Make<float>(3.0f, 4.0f));

    EXPECT_FLOAT_EQ(0.6f, v.X);
    EXPECT_FLOAT_EQ(0.8f, v.Y);
}

TEST(VectorNormalizeTest, AcceptsIntegerInput)
{
    auto v = Normalize(Make<int>(0, 0, 5));

    EXPECT_FLOAT_EQ(0.0f, v.X);
    EXPECT_FLOAT_EQ(0.0f, v.Y);
    EXPECT_FLOAT_EQ(1.0f, v.Z);
}

TEST(VectorNormalizeTest, LeavesUnitVectorUnchanged)
{
    auto v = Normalize(world::Up);

    EXPECT_FLOAT_EQ(0.0f, v.X);
    EXPECT_FLOAT_EQ(1.0f, v.Y);
    EXPECT_FLOAT_EQ(0.0f, v.Z);
}

TEST(VectorNormalizeTest, AlwaysReturnsFloatVector)
{
    auto fromInt    = Normalize(Make<int>(0, 5));
    auto fromDouble = Normalize(Make<double>(0.0, 5.0));

    EXPECT_TRUE((std::is_same_v<decltype(fromInt), Vec2>));
    EXPECT_TRUE((std::is_same_v<decltype(fromDouble), Vec2>));
}

TEST(VectorPerpTest, CounterClockwiseRotatesQuarterTurn)
{
    auto p = PerpCCW(Make<int>(1, 0));

    EXPECT_EQ(0, p.X);
    EXPECT_EQ(1, p.Y);
}

TEST(VectorPerpTest, ClockwiseRotatesQuarterTurn)
{
    auto p = PerpCW(Make<int>(1, 0));

    EXPECT_EQ(0, p.X);
    EXPECT_EQ(-1, p.Y);
}

TEST(VectorPerpTest, ResultIsOrthogonalToInput)
{
    auto v = Make<int>(3, 4);

    EXPECT_EQ(0, Dot(v, PerpCCW(v)));
    EXPECT_EQ(0, Dot(v, PerpCW(v)));
}

TEST(VectorPerpTest, RotationDirectionsHaveOppositeSign)
{
    auto v = Make<int>(3, 4);

    EXPECT_GT(Cross(v, PerpCCW(v)), 0);
    EXPECT_LT(Cross(v, PerpCW(v)), 0);
}

TEST(VectorPerpTest, PreservesLength)
{
    auto v = Make<float>(3.0f, 4.0f);

    EXPECT_FLOAT_EQ(Length(v), Length(PerpCCW(v)));
    EXPECT_FLOAT_EQ(Length(v), Length(PerpCW(v)));
}

TEST(VectorPerpTest, AppliedFourTimesReturnsOriginal)
{
    auto v = Make<int>(3, 4);

    auto r = PerpCCW(PerpCCW(PerpCCW(PerpCCW(v))));

    EXPECT_EQ(v.X, r.X);
    EXPECT_EQ(v.Y, r.Y);
}

TEST(VectorShorthandTest, MatchTheirTemplateCounterparts)
{
    EXPECT_TRUE(NearlyEqual(vec2::Zero, Zero<float, 2>));
    EXPECT_TRUE(NearlyEqual(vec2::One, One<float, 2>));
    EXPECT_TRUE(NearlyEqual(vec2::UnitX, UnitX<float, 2>));
    EXPECT_TRUE(NearlyEqual(vec2::UnitY, UnitY<float, 2>));

    EXPECT_TRUE(NearlyEqual(vec3::Zero, Zero<float, 3>));
    EXPECT_TRUE(NearlyEqual(vec3::One, One<float, 3>));
    EXPECT_TRUE(NearlyEqual(vec3::UnitX, UnitX<float, 3>));
    EXPECT_TRUE(NearlyEqual(vec3::UnitY, UnitY<float, 3>));
    EXPECT_TRUE(NearlyEqual(vec3::UnitZ, UnitZ<float, 3>));

    EXPECT_TRUE(NearlyEqual(vec4::Zero, Zero<float, 4>));
    EXPECT_TRUE(NearlyEqual(vec4::One, One<float, 4>));
    EXPECT_TRUE(NearlyEqual(vec4::UnitX, UnitX<float, 4>));
    EXPECT_TRUE(NearlyEqual(vec4::UnitY, UnitY<float, 4>));
    EXPECT_TRUE(NearlyEqual(vec4::UnitZ, UnitZ<float, 4>));
    EXPECT_TRUE(NearlyEqual(vec4::UnitW, UnitW<float, 4>));
}

TEST(VectorShorthandTest, SelectTheExpectedAxis)
{
    EXPECT_FLOAT_EQ(1.0f, vec3::UnitY.Y);
    EXPECT_FLOAT_EQ(0.0f, vec3::UnitY.X);
    EXPECT_FLOAT_EQ(0.0f, vec3::UnitY.Z);

    EXPECT_FLOAT_EQ(1.0f, vec4::UnitW.W);
    EXPECT_FLOAT_EQ(0.0f, vec4::UnitW.Z);
}

namespace
{
    static_assert(std::is_same_v<decltype(vec2::Zero), const Vec2>);
    static_assert(std::is_same_v<decltype(vec3::UnitZ), const Vec3>);
    static_assert(std::is_same_v<decltype(vec4::UnitW), const Vec4>);

    static_assert(vec2::One.Data[0] == 1.0f && vec2::One.Data[1] == 1.0f);
    static_assert(vec3::UnitZ.Data[2] == 1.0f && vec3::UnitZ.Data[0] == 0.0f);
    static_assert(vec4::UnitW.Data[3] == 1.0f && vec4::UnitW.Data[2] == 0.0f);
    static_assert(vec4::Zero.Data[3] == 0.0f);
}

namespace
{
    constexpr auto g_A   = Make<int>(1, 2, 3);
    constexpr auto g_B   = Make<int>(4, 5, 6);
    constexpr auto g_Sum = g_A + g_B;

    static_assert(g_Sum.Data[0] == 5);
    static_assert(g_Sum.Data[1] == 7);
    static_assert(g_Sum.Data[2] == 9);

    static_assert(Dot(g_A, g_B) == 32);
    static_assert(LengthSquared(g_A) == 14);
    static_assert(DistanceSquared(g_A, g_B) == 27);

    static_assert(Zero<int, 3>.Data[0] == 0);
    static_assert(One<int, 3>.Data[1] == 1);
    static_assert(UnitX<int, 3>.Data[0] == 1);
    static_assert(UnitY<int, 3>.Data[1] == 1);
    static_assert(UnitZ<int, 3>.Data[2] == 1);

    static_assert(Fill<int, 3>(7).Data[2] == 7);
    static_assert(Axis<int, 3>(2, 4).Data[2] == 4);

    static_assert(world::Right.Data[0] == 1.0f);
    static_assert(world::Left.Data[0] == -1.0f);
    static_assert(world::Up.Data[1] == 1.0f);
    static_assert(world::Down.Data[1] == -1.0f);
    static_assert(world::Forward.Data[2] == -1.0f);
    static_assert(world::Back.Data[2] == 1.0f);
}

TEST(VectorConstexprTest, ExpressionsEvaluateAtCompileTime)
{
    SUCCEED();
}
