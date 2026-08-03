#include <gtest/gtest.h>

#include <math/math_utils.h>
#include <math/matrix.h>
#include <math/quat.h>
#include <math/vector.h>

#include <cmath>

using namespace cw;
using namespace cw::math;
using namespace cw::math::quat;

namespace
{
    constexpr float HalfPi = Pi * 0.5f;
    constexpr float QuarterPi = Pi * 0.25f;

    constexpr float Tolerance = 1e-5f;

    constexpr Quaternion I{1.0f, 0.0f, 0.0f, 0.0f};
    constexpr Quaternion J{0.0f, 1.0f, 0.0f, 0.0f};
    constexpr Quaternion K{0.0f, 0.0f, 1.0f, 0.0f};

    bool Same(const Quaternion& lhs, const Quaternion& rhs, float tolerance = Tolerance)
    {
        for (usize i = 0; i < 4; ++i)
        {
            if (!math::NearlyEqual(lhs.Data[i], rhs.Data[i], tolerance))
            {
                return false;
            }
        }

        return true;
    }

    Quaternion MakeArbitrary()
    {
        return Euler(0.3f, -0.7f, 1.1f);
    }

    Quaternion MakeOtherArbitrary()
    {
        return Euler(-1.2f, 0.4f, 0.9f);
    }

    Vec3 MakeArbitraryVector()
    {
        return Vec3{0.3f, -0.4f, 0.9f};
    }
}

TEST(QuaternionStorageTest, NamedAccessorsAliasData)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(q.Data[0], q.X);
    EXPECT_FLOAT_EQ(q.Data[1], q.Y);
    EXPECT_FLOAT_EQ(q.Data[2], q.Z);
    EXPECT_FLOAT_EQ(q.Data[3], q.W);

    q.Z = 7.0f;

    EXPECT_FLOAT_EQ(7.0f, q.Data[2]);
}

TEST(QuaternionStorageTest, HasNoPaddingAroundComponents)
{
    EXPECT_EQ(sizeof(float) * 4, sizeof(Quaternion));
}

TEST(QuaternionStorageTest, IdentityIsScalarOne)
{
    EXPECT_FLOAT_EQ(0.0f, Identity.X);
    EXPECT_FLOAT_EQ(0.0f, Identity.Y);
    EXPECT_FLOAT_EQ(0.0f, Identity.Z);
    EXPECT_FLOAT_EQ(1.0f, Identity.W);
}

TEST(QuaternionLengthTest, IdentityHasUnitLength)
{
    EXPECT_FLOAT_EQ(1.0f, Length(Identity));
    EXPECT_FLOAT_EQ(1.0f, LengthSquared(Identity));
}

TEST(QuaternionLengthTest, SumsSquaresOfAllFourComponents)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, -4.0f};

    EXPECT_FLOAT_EQ(30.0f, LengthSquared(q));
    EXPECT_FLOAT_EQ(std::sqrt(30.0f), Length(q));
}

TEST(QuaternionLengthTest, ScalingScalesLengthLinearly)
{
    const Quaternion q = MakeArbitrary();

    EXPECT_NEAR(Length(q) * 3.0f, Length(q * 3.0f), 1e-4f);
}

TEST(QuaternionIsNormalizedTest, AcceptsUnitQuaternions)
{
    EXPECT_TRUE(IsNormalized(Identity));
    EXPECT_TRUE(IsNormalized(I));
    EXPECT_TRUE(IsNormalized(MakeArbitrary()));
    EXPECT_TRUE(IsNormalized(FromAxisAngle(Vec3{1.0f, 2.0f, 3.0f}, 0.7f)));
}

TEST(QuaternionIsNormalizedTest, RejectsScaledAndZeroQuaternions)
{
    EXPECT_FALSE(IsNormalized(Identity * 2.0f));
    EXPECT_FALSE(IsNormalized(Quaternion{0.0f, 0.0f, 0.0f, 0.0f}));
}

TEST(QuaternionIsNormalizedTest, HonoursExplicitTolerance)
{
    const Quaternion q = Identity * 1.001f;

    EXPECT_FALSE(IsNormalized(q));
    EXPECT_TRUE(IsNormalized(q, 1e-2f));
}

TEST(QuaternionNormalizeTest, ProducesUnitLength)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, -4.0f};

    EXPECT_NEAR(1.0f, Length(Normalize(q)), Tolerance);
    EXPECT_TRUE(IsNormalized(Normalize(q)));
}

TEST(QuaternionNormalizeTest, LeavesUnitQuaternionUnchanged)
{
    const Quaternion q = MakeArbitrary();

    EXPECT_TRUE(Same(q, Normalize(q), 1e-4f));
}

TEST(QuaternionNormalizeTest, IgnoresMagnitudeOfInput)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, -4.0f};

    EXPECT_TRUE(Same(Normalize(q), Normalize(q * 100.0f), 1e-4f));
}

TEST(QuaternionNormalizeTest, PreservesSign)
{
    const Quaternion q{0.0f, 0.0f, 0.0f, -2.0f};

    EXPECT_TRUE(Same(Quaternion{0.0f, 0.0f, 0.0f, -1.0f}, Normalize(q)));
}

TEST(QuaternionArithmeticTest, ScalesFromEitherSide)
{
    const Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};
    const Quaternion expected{2.0f, 4.0f, 6.0f, 8.0f};

    EXPECT_TRUE(Same(expected, q * 2.0f));
    EXPECT_TRUE(Same(expected, 2.0f * q));
}

TEST(QuaternionArithmeticTest, DividesByScalar)
{
    const Quaternion q{2.0f, 4.0f, 6.0f, 8.0f};

    EXPECT_TRUE(Same(Quaternion{1.0f, 2.0f, 3.0f, 4.0f}, q / 2.0f));
}

TEST(QuaternionArithmeticTest, AddsAndSubtractsComponentWise)
{
    const Quaternion lhs{1.0f, 2.0f, 3.0f, 4.0f};
    const Quaternion rhs{5.0f, -6.0f, 7.0f, 8.0f};

    EXPECT_TRUE(Same(Quaternion{6.0f, -4.0f, 10.0f, 12.0f}, lhs + rhs));
    EXPECT_TRUE(Same(Quaternion{-4.0f, 8.0f, -4.0f, -4.0f}, lhs - rhs));
}

TEST(QuaternionArithmeticTest, NegatesEveryComponent)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, -4.0f};

    EXPECT_TRUE(Same(Quaternion{-1.0f, 2.0f, -3.0f, 4.0f}, -q));
    EXPECT_TRUE(Same(q, -(-q)));
}

TEST(QuaternionArithmeticTest, BinaryOperatorsLeaveOperandsUntouched)
{
    const Quaternion lhs{1.0f, 2.0f, 3.0f, 4.0f};
    const Quaternion rhs{5.0f, 6.0f, 7.0f, 8.0f};

    const Quaternion sum = lhs + rhs;

    EXPECT_TRUE(Same(Quaternion{1.0f, 2.0f, 3.0f, 4.0f}, lhs));
    EXPECT_TRUE(Same(Quaternion{5.0f, 6.0f, 7.0f, 8.0f}, rhs));
    EXPECT_TRUE(Same(Quaternion{6.0f, 8.0f, 10.0f, 12.0f}, sum));
}

TEST(QuaternionProductTest, IdentityIsNeutralOnBothSides)
{
    const Quaternion q = MakeArbitrary();

    EXPECT_TRUE(Same(q, q * Identity));
    EXPECT_TRUE(Same(q, Identity * q));
}

TEST(QuaternionProductTest, FollowsHamiltonRulesForBasisElements)
{
    EXPECT_TRUE(Same(K, I * J));
    EXPECT_TRUE(Same(I, J * K));
    EXPECT_TRUE(Same(J, K * I));

    EXPECT_TRUE(Same(-K, J * I));
    EXPECT_TRUE(Same(-I, K * J));
    EXPECT_TRUE(Same(-J, I * K));
}

TEST(QuaternionProductTest, BasisElementsSquareToMinusOne)
{
    EXPECT_TRUE(Same(-Identity, I * I));
    EXPECT_TRUE(Same(-Identity, J * J));
    EXPECT_TRUE(Same(-Identity, K * K));
    EXPECT_TRUE(Same(-Identity, I * J * K));
}

TEST(QuaternionProductTest, IsNotCommutative)
{
    const Quaternion lhs = MakeArbitrary();
    const Quaternion rhs = MakeOtherArbitrary();

    EXPECT_FALSE(Same(lhs * rhs, rhs * lhs));
}

TEST(QuaternionProductTest, IsAssociative)
{
    const Quaternion a = MakeArbitrary();
    const Quaternion b = MakeOtherArbitrary();
    const Quaternion c = FromAxisAngle(Vec3{1.0f, 1.0f, 0.0f}, 0.6f);

    EXPECT_TRUE(Same((a * b) * c, a * (b * c), 1e-4f));
}

TEST(QuaternionProductTest, MultipliesLengths)
{
    const Quaternion lhs{1.0f, 2.0f, 3.0f, 4.0f};
    const Quaternion rhs{-2.0f, 0.5f, 1.0f, 3.0f};

    EXPECT_NEAR(Length(lhs) * Length(rhs), Length(lhs * rhs), 1e-3f);
}

TEST(QuaternionProductTest, KeepsUnitQuaternionsUnit)
{
    const Quaternion product = MakeArbitrary() * MakeOtherArbitrary();

    EXPECT_TRUE(IsNormalized(product));
}

TEST(QuaternionProductTest, DistributesOverAddition)
{
    const Quaternion a = MakeArbitrary();
    const Quaternion b = MakeOtherArbitrary();
    const Quaternion c{1.0f, -1.0f, 0.5f, 2.0f};

    EXPECT_TRUE(Same(a * (b + c), a * b + a * c, 1e-4f));
}

TEST(QuaternionConjugateTest, NegatesVectorPartOnly)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, 4.0f};

    EXPECT_TRUE(Same(Quaternion{-1.0f, 2.0f, -3.0f, 4.0f}, Conjugate(q)));
}

TEST(QuaternionConjugateTest, AppliedTwiceReturnsOriginal)
{
    const Quaternion q = MakeArbitrary();

    EXPECT_TRUE(Same(q, Conjugate(Conjugate(q))));
}

TEST(QuaternionConjugateTest, PreservesLength)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(Length(q), Length(Conjugate(q)));
}

TEST(QuaternionConjugateTest, ReversesProductOrder)
{
    const Quaternion a = MakeArbitrary();
    const Quaternion b = MakeOtherArbitrary();

    EXPECT_TRUE(Same(Conjugate(a * b), Conjugate(b) * Conjugate(a), 1e-4f));
}

TEST(QuaternionConjugateTest, TimesItselfGivesLengthSquared)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, 4.0f};

    const Quaternion product = q * Conjugate(q);

    EXPECT_NEAR(LengthSquared(q), product.W, 1e-3f);
    EXPECT_NEAR(0.0f, product.X, 1e-4f);
    EXPECT_NEAR(0.0f, product.Y, 1e-4f);
    EXPECT_NEAR(0.0f, product.Z, 1e-4f);
}

TEST(QuaternionInverseTest, MatchesConjugateForUnitQuaternions)
{
    const Quaternion q = MakeArbitrary();

    EXPECT_TRUE(Same(Conjugate(q), Inverse(q), 1e-4f));
}

TEST(QuaternionInverseTest, ProductWithInverseIsIdentity)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, 4.0f};

    EXPECT_TRUE(Same(Identity, q * Inverse(q), 1e-4f));
    EXPECT_TRUE(Same(Identity, Inverse(q) * q, 1e-4f));
}

TEST(QuaternionInverseTest, AppliedTwiceReturnsOriginal)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, 4.0f};

    EXPECT_TRUE(Same(q, Inverse(Inverse(q)), 1e-4f));
}

TEST(QuaternionInverseTest, ScalesInverselyWithMagnitude)
{
    const Quaternion q = MakeArbitrary();

    EXPECT_TRUE(Same(Inverse(q) / 2.0f, Inverse(q * 2.0f), 1e-4f));
}

TEST(QuaternionDotTest, SelfDotEqualsLengthSquared)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(LengthSquared(q), Dot(q, q));
}

TEST(QuaternionDotTest, IsCommutative)
{
    const Quaternion lhs = MakeArbitrary();
    const Quaternion rhs = MakeOtherArbitrary();

    EXPECT_FLOAT_EQ(Dot(lhs, rhs), Dot(rhs, lhs));
}

TEST(QuaternionDotTest, OrthogonalBasisElementsGiveZero)
{
    EXPECT_FLOAT_EQ(0.0f, Dot(I, J));
    EXPECT_FLOAT_EQ(0.0f, Dot(J, K));
    EXPECT_FLOAT_EQ(0.0f, Dot(K, Identity));
}

TEST(QuaternionDotTest, OppositeQuaternionsGiveNegativeLengthSquared)
{
    const Quaternion q{1.0f, -2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(-LengthSquared(q), Dot(q, -q));
}

TEST(QuaternionFromAxisAngleTest, ZeroAngleGivesIdentity)
{
    EXPECT_TRUE(Same(Identity, FromAxisAngle(vec3::UnitY, 0.0f)));
}

TEST(QuaternionFromAxisAngleTest, EncodesHalfAngle)
{
    const Quaternion q = FromAxisAngle(vec3::UnitZ, HalfPi);

    EXPECT_NEAR(0.0f, q.X, Tolerance);
    EXPECT_NEAR(0.0f, q.Y, Tolerance);
    EXPECT_NEAR(std::sin(QuarterPi), q.Z, Tolerance);
    EXPECT_NEAR(std::cos(QuarterPi), q.W, Tolerance);
}

TEST(QuaternionFromAxisAngleTest, ProducesUnitQuaternion)
{
    EXPECT_TRUE(IsNormalized(FromAxisAngle(Vec3{1.0f, 2.0f, -3.0f}, 2.4f)));
}

TEST(QuaternionFromAxisAngleTest, NormalizesTheAxis)
{
    const Vec3 axis{0.0f, 0.0f, 5.0f};

    EXPECT_TRUE(Same(FromAxisAngle(vec3::UnitZ, 0.8f), FromAxisAngle(axis, 0.8f), 1e-5f));
}

TEST(QuaternionFromAxisAngleTest, FullTurnGivesNegatedIdentity)
{
    EXPECT_TRUE(Same(-Identity, FromAxisAngle(vec3::UnitX, Pi * 2.0f), 1e-4f));
}

TEST(QuaternionFromAxisAngleTest, OppositeAngleGivesConjugate)
{
    const Vec3 axis{1.0f, -2.0f, 0.5f};

    EXPECT_TRUE(Same(Conjugate(FromAxisAngle(axis, 1.3f)), FromAxisAngle(axis, -1.3f), 1e-5f));
}

TEST(QuaternionEulerTest, ZeroAnglesGiveIdentity)
{
    EXPECT_TRUE(Same(Identity, Euler(0.0f, 0.0f, 0.0f)));
}

TEST(QuaternionEulerTest, SingleAxisMatchesFromAxisAngle)
{
    EXPECT_TRUE(Same(FromAxisAngle(vec3::UnitX, 0.7f), Euler(0.7f, 0.0f, 0.0f), 1e-5f));
    EXPECT_TRUE(Same(FromAxisAngle(vec3::UnitY, 0.7f), Euler(0.0f, 0.7f, 0.0f), 1e-5f));
    EXPECT_TRUE(Same(FromAxisAngle(vec3::UnitZ, 0.7f), Euler(0.0f, 0.0f, 0.7f), 1e-5f));
}

TEST(QuaternionEulerTest, AppliesRotationsInXThenYThenZOrder)
{
    const float x = 0.3f;
    const float y = -0.7f;
    const float z = 1.1f;

    const Quaternion composed =
        Euler(0.0f, 0.0f, z) * Euler(0.0f, y, 0.0f) * Euler(x, 0.0f, 0.0f);

    EXPECT_TRUE(Same(composed, Euler(x, y, z), 1e-4f));
}

TEST(QuaternionEulerTest, ProducesUnitQuaternion)
{
    EXPECT_TRUE(IsNormalized(Euler(2.1f, -0.4f, 3.0f)));
}

TEST(QuaternionRotateVectorTest, IdentityLeavesVectorUnchanged)
{
    const Vec3 v = MakeArbitraryVector();

    EXPECT_TRUE(NearlyEqual(v, RotateVector(Identity, v), Tolerance));
}

TEST(QuaternionRotateVectorTest, TurnsAxesByNinetyDegreesAboutZ)
{
    const Quaternion q = FromAxisAngle(vec3::UnitZ, HalfPi);

    EXPECT_TRUE(NearlyEqual(vec3::UnitY, RotateVector(q, vec3::UnitX), 1e-5f));
    EXPECT_TRUE(NearlyEqual(vec3::UnitX * -1.0f, RotateVector(q, vec3::UnitY), 1e-5f));
}

TEST(QuaternionRotateVectorTest, TurnsAxesByNinetyDegreesAboutXAndY)
{
    const Quaternion aboutX = FromAxisAngle(vec3::UnitX, HalfPi);
    const Quaternion aboutY = FromAxisAngle(vec3::UnitY, HalfPi);

    EXPECT_TRUE(NearlyEqual(vec3::UnitZ, RotateVector(aboutX, vec3::UnitY), 1e-5f));
    EXPECT_TRUE(NearlyEqual(vec3::UnitX, RotateVector(aboutY, vec3::UnitZ), 1e-5f));
}

TEST(QuaternionRotateVectorTest, LeavesTheRotationAxisFixed)
{
    const Vec3 axis = vec::Normalize(Vec3{1.0f, 2.0f, -1.0f});
    const Quaternion q = FromAxisAngle(axis, 1.4f);

    EXPECT_TRUE(NearlyEqual(axis, RotateVector(q, axis), 1e-4f));
}

TEST(QuaternionRotateVectorTest, PreservesLength)
{
    const Vec3 v = MakeArbitraryVector();
    const Quaternion q = MakeArbitrary();

    EXPECT_NEAR(vec::Length(v), vec::Length(RotateVector(q, v)), 1e-5f);
}

TEST(QuaternionRotateVectorTest, ComposesInTheSameOrderAsTheProduct)
{
    const Quaternion a = MakeArbitrary();
    const Quaternion b = MakeOtherArbitrary();
    const Vec3 v = MakeArbitraryVector();

    EXPECT_TRUE(NearlyEqual(RotateVector(a * b, v), RotateVector(a, RotateVector(b, v)), 1e-4f));
}

TEST(QuaternionRotateVectorTest, ConjugateUndoesTheRotation)
{
    const Quaternion q = MakeArbitrary();
    const Vec3 v = MakeArbitraryVector();

    EXPECT_TRUE(NearlyEqual(v, RotateVector(Conjugate(q), RotateVector(q, v)), 1e-4f));
}

TEST(QuaternionRotateVectorTest, OppositeQuaternionsRotateAlike)
{
    const Quaternion q = MakeArbitrary();
    const Vec3 v = MakeArbitraryVector();

    EXPECT_TRUE(NearlyEqual(RotateVector(q, v), RotateVector(-q, v), 1e-4f));
}

TEST(QuaternionToMatrixTest, IdentityGivesIdentityMatrices)
{
    EXPECT_TRUE(NearlyEqual(mat3::Identity, ToMat3(Identity)));
    EXPECT_TRUE(NearlyEqual(mat4::Identity, ToMat4(Identity)));
}

TEST(QuaternionToMatrixTest, Mat3AgreesWithRotateVector)
{
    const Quaternion q = MakeArbitrary();
    const Vec3 v = MakeArbitraryVector();

    EXPECT_TRUE(NearlyEqual(RotateVector(q, v), ToMat3(q) * v, 1e-4f));
}

TEST(QuaternionToMatrixTest, Mat3IsARotation)
{
    const Mat3 rotation = ToMat3(MakeArbitrary());

    EXPECT_TRUE(NearlyEqual(mat3::Identity, rotation * mat::Transpose(rotation), 1e-4f));
    EXPECT_NEAR(1.0f, mat::Determinant(rotation), 1e-4f);
}

TEST(QuaternionToMatrixTest, ConjugateGivesTransposedMatrix)
{
    const Quaternion q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(mat::Transpose(ToMat3(q)), ToMat3(Conjugate(q)), 1e-5f));
}

TEST(QuaternionToMatrixTest, ProductMapsToMatrixProduct)
{
    const Quaternion a = MakeArbitrary();
    const Quaternion b = MakeOtherArbitrary();

    EXPECT_TRUE(NearlyEqual(ToMat3(a * b), ToMat3(a) * ToMat3(b), 1e-4f));
}

TEST(QuaternionToMatrixTest, Mat4EmbedsMat3InTheUpperLeftBlock)
{
    const Quaternion q = MakeArbitrary();

    const Mat3 three = ToMat3(q);
    const Mat4 four = ToMat4(q);

    for (usize row = 0; row < 3; ++row)
    {
        for (usize column = 0; column < 3; ++column)
        {
            EXPECT_NEAR(mat::At(three, row, column), mat::At(four, row, column), 1e-5f);
        }
    }
}

TEST(QuaternionToMatrixTest, Mat4HasNoTranslationOrProjection)
{
    const Mat4 four = ToMat4(MakeArbitrary());

    for (usize i = 0; i < 3; ++i)
    {
        EXPECT_FLOAT_EQ(0.0f, mat::At(four, i, 3));
        EXPECT_FLOAT_EQ(0.0f, mat::At(four, 3, i));
    }

    EXPECT_FLOAT_EQ(1.0f, mat::At(four, 3, 3));
}

TEST(QuaternionSlerpTest, ReturnsEndpointsAtZeroAndOne)
{
    const Quaternion q0 = FromAxisAngle(vec3::UnitZ, 0.2f);
    const Quaternion q1 = FromAxisAngle(vec3::UnitY, 1.0f);

    ASSERT_GT(Dot(q0, q1), 0.0f);

    EXPECT_TRUE(Same(q0, Slerp(q0, q1, 0.0f), 1e-4f));
    EXPECT_TRUE(Same(q1, Slerp(q0, q1, 1.0f), 1e-4f));
}

TEST(QuaternionSlerpTest, ReturnsEndpointRotationsRegardlessOfSign)
{
    const Quaternion q0 = MakeArbitrary();
    const Quaternion q1 = MakeOtherArbitrary();
    const Vec3 v = MakeArbitraryVector();

    EXPECT_TRUE(NearlyEqual(RotateVector(q0, v), RotateVector(Slerp(q0, q1, 0.0f), v), 1e-4f));
    EXPECT_TRUE(NearlyEqual(RotateVector(q1, v), RotateVector(Slerp(q0, q1, 1.0f), v), 1e-4f));
}

TEST(QuaternionSlerpTest, StaysNormalized)
{
    const Quaternion q0 = MakeArbitrary();
    const Quaternion q1 = MakeOtherArbitrary();

    for (int32 step = 0; step <= 10; ++step)
    {
        const float t = static_cast<float>(step) / 10.0f;

        EXPECT_TRUE(IsNormalized(Slerp(q0, q1, t)));
    }
}

TEST(QuaternionSlerpTest, MovesAtConstantAngularVelocity)
{
    const Quaternion q0 = FromAxisAngle(vec3::UnitZ, 0.0f);
    const Quaternion q1 = FromAxisAngle(vec3::UnitZ, HalfPi);

    const Quaternion middle = Slerp(q0, q1, 0.5f);

    EXPECT_NEAR(Abs(Dot(q0, middle)), Abs(Dot(middle, q1)), 1e-4f);
    EXPECT_TRUE(Same(FromAxisAngle(vec3::UnitZ, QuarterPi), middle, 1e-4f));
}

TEST(QuaternionSlerpTest, IsSymmetricInItsEndpoints)
{
    const Quaternion q0 = MakeArbitrary();
    const Quaternion q1 = MakeOtherArbitrary();
    const Vec3 v = MakeArbitraryVector();

    const Vec3 forward = RotateVector(Slerp(q0, q1, 0.3f), v);
    const Vec3 backward = RotateVector(Slerp(q1, q0, 0.7f), v);

    EXPECT_TRUE(NearlyEqual(forward, backward, 1e-4f));
}

TEST(QuaternionSlerpTest, TakesTheShortestPath)
{
    const Quaternion q0 = MakeArbitrary();
    const Quaternion q1 = MakeOtherArbitrary();

    EXPECT_TRUE(Same(Slerp(q0, q1, 0.4f), Slerp(q0, -q1, 0.4f), 1e-4f));
    EXPECT_GE(Dot(q0, Slerp(q0, -q1, 0.4f)), 0.0f);
}

TEST(QuaternionSlerpTest, HandlesIdenticalEndpoints)
{
    const Quaternion q = MakeArbitrary();

    EXPECT_TRUE(Same(q, Slerp(q, q, 0.5f), 1e-4f));
}

TEST(QuaternionSlerpTest, FallsBackToLinearPathForNearbyQuaternions)
{
    const Quaternion q0 = FromAxisAngle(vec3::UnitZ, 0.0f);
    const Quaternion q1 = FromAxisAngle(vec3::UnitZ, 1e-3f);

    ASSERT_GT(Dot(q0, q1), 0.9995f);

    const Quaternion middle = Slerp(q0, q1, 0.5f);

    EXPECT_TRUE(IsNormalized(middle));
    EXPECT_TRUE(Same(FromAxisAngle(vec3::UnitZ, 5e-4f), middle, 1e-4f));
}

TEST(QuaternionSlerpTest, InterpolatedRotationActsOnVectors)
{
    const Quaternion q0 = Identity;
    const Quaternion q1 = FromAxisAngle(vec3::UnitZ, HalfPi);

    const Vec3 rotated = RotateVector(Slerp(q0, q1, 0.5f), vec3::UnitX);

    EXPECT_TRUE(NearlyEqual(Vec3{std::cos(QuarterPi), std::sin(QuarterPi), 0.0f}, rotated, 1e-4f));
}
