#include <gtest/gtest.h>

#include <math/quat.h>

using namespace cw;
using namespace cw::math;
using namespace cw::math::quat;

namespace
{
    constexpr float Pi        = 3.14159265358979323846f;
    constexpr float HalfPi    = Pi * 0.5f;
    constexpr float QuarterPi = Pi * 0.25f;

    constexpr float Tolerance = 1e-5f;

    constexpr Quaternion I{1.0f, 0.0f, 0.0f, 0.0f};
    constexpr Quaternion J{0.0f, 1.0f, 0.0f, 0.0f};
    constexpr Quaternion K{0.0f, 0.0f, 1.0f, 0.0f};

    bool NearlyEqual(const Quaternion& lhs, const Quaternion& rhs, float tolerance = Tolerance)
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
}

TEST(QuaternionStorageTest, HasNoPaddingAroundComponents)
{
    EXPECT_EQ(4 * sizeof(float), sizeof(Quaternion));
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
    EXPECT_FLOAT_EQ(1.0f, LengthSquared(Identity));
    EXPECT_FLOAT_EQ(1.0f, Length(Identity));
}

TEST(QuaternionLengthTest, SumsSquaresOfAllFourComponents)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(30.0f, LengthSquared(q));
    EXPECT_FLOAT_EQ(std::sqrt(30.0f), Length(q));
}

TEST(QuaternionLengthTest, SquaredMatchesLengthTimesLength)
{
    Quaternion q{1.0f, 1.0f, 1.0f, 1.0f};

    EXPECT_FLOAT_EQ(2.0f, Length(q));
    EXPECT_FLOAT_EQ(Length(q) * Length(q), LengthSquared(q));
}

TEST(QuaternionLengthTest, ScalingScalesLengthLinearly)
{
    auto q = MakeArbitrary();

    EXPECT_FLOAT_EQ(3.0f * Length(q), Length(q * 3.0f));
}

TEST(QuaternionIsNormalizedTest, AcceptsUnitQuaternions)
{
    EXPECT_TRUE(IsNormalized(Identity));
    EXPECT_TRUE(IsNormalized(I));
    EXPECT_TRUE(IsNormalized(J));
    EXPECT_TRUE(IsNormalized(K));
    EXPECT_TRUE(IsNormalized(MakeArbitrary()));
}

TEST(QuaternionIsNormalizedTest, RejectsScaledAndZeroQuaternions)
{
    EXPECT_FALSE(IsNormalized(Identity * 1.01f));
    EXPECT_FALSE(IsNormalized(Identity * 0.5f));
    EXPECT_FALSE(IsNormalized(Quaternion{0.0f, 0.0f, 0.0f, 0.0f}));
}

TEST(QuaternionIsNormalizedTest, HonoursExplicitTolerance)
{
    auto q = Identity * 1.001f;

    EXPECT_FALSE(IsNormalized(q));
    EXPECT_TRUE(IsNormalized(q, 0.01f));
}

TEST(QuaternionNormalizeTest, ProducesUnitLength)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(1.0f, Length(Normalize(q)));
    EXPECT_TRUE(IsNormalized(Normalize(q)));
}

TEST(QuaternionNormalizeTest, LeavesUnitQuaternionUnchanged)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(q, Normalize(q)));
}

TEST(QuaternionNormalizeTest, IgnoresMagnitudeOfInput)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(Normalize(q), Normalize(q * 7.0f)));
    EXPECT_TRUE(NearlyEqual(Normalize(q), Normalize(q * 0.01f)));
}

TEST(QuaternionNormalizeTest, PreservesSign)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(-Normalize(q), Normalize(-q)));
}

TEST(QuaternionArithmeticTest, ScalesFromEitherSide)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_TRUE(NearlyEqual(Quaternion{2.0f, 4.0f, 6.0f, 8.0f}, q * 2.0f));
    EXPECT_TRUE(NearlyEqual(q * 2.0f, 2.0f * q));
}

TEST(QuaternionArithmeticTest, DividesByScalar)
{
    Quaternion q{2.0f, 4.0f, 6.0f, 8.0f};

    EXPECT_TRUE(NearlyEqual(Quaternion{1.0f, 2.0f, 3.0f, 4.0f}, q / 2.0f));
    EXPECT_TRUE(NearlyEqual(q * 0.5f, q / 2.0f));
}

TEST(QuaternionArithmeticTest, AddsAndSubtractsComponentWise)
{
    Quaternion a{1.0f, 2.0f, 3.0f, 4.0f};
    Quaternion b{5.0f, 6.0f, 7.0f, 8.0f};

    EXPECT_TRUE(NearlyEqual(Quaternion{6.0f, 8.0f, 10.0f, 12.0f}, a + b));
    EXPECT_TRUE(NearlyEqual(Quaternion{-4.0f, -4.0f, -4.0f, -4.0f}, a - b));
    EXPECT_TRUE(NearlyEqual(Quaternion{0.0f, 0.0f, 0.0f, 0.0f}, a - a));
}

TEST(QuaternionArithmeticTest, NegatesEveryComponent)
{
    Quaternion q{1.0f, -2.0f, 3.0f, -4.0f};

    EXPECT_TRUE(NearlyEqual(Quaternion{-1.0f, 2.0f, -3.0f, 4.0f}, -q));
    EXPECT_TRUE(NearlyEqual(q, -(-q)));
    EXPECT_TRUE(NearlyEqual(q * -1.0f, -q));
}

TEST(QuaternionArithmeticTest, BinaryOperatorsLeaveOperandsUntouched)
{
    Quaternion a{1.0f, 2.0f, 3.0f, 4.0f};
    Quaternion b{5.0f, 6.0f, 7.0f, 8.0f};

    auto c = a + b;

    EXPECT_FLOAT_EQ(1.0f, a.X);
    EXPECT_FLOAT_EQ(5.0f, b.X);
    EXPECT_FLOAT_EQ(6.0f, c.X);
}

TEST(QuaternionProductTest, IdentityIsNeutralOnBothSides)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(q, q * Identity));
    EXPECT_TRUE(NearlyEqual(q, Identity * q));
}

TEST(QuaternionProductTest, FollowsHamiltonRulesForBasisElements)
{
    EXPECT_TRUE(NearlyEqual(K, I * J));
    EXPECT_TRUE(NearlyEqual(I, J * K));
    EXPECT_TRUE(NearlyEqual(J, K * I));

    EXPECT_TRUE(NearlyEqual(-K, J * I));
    EXPECT_TRUE(NearlyEqual(-I, K * J));
    EXPECT_TRUE(NearlyEqual(-J, I * K));
}

TEST(QuaternionProductTest, BasisElementsSquareToMinusOne)
{
    Quaternion minusOne{0.0f, 0.0f, 0.0f, -1.0f};

    EXPECT_TRUE(NearlyEqual(minusOne, I * I));
    EXPECT_TRUE(NearlyEqual(minusOne, J * J));
    EXPECT_TRUE(NearlyEqual(minusOne, K * K));
    EXPECT_TRUE(NearlyEqual(minusOne, I * J * K));
}

TEST(QuaternionProductTest, IsNotCommutative)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    EXPECT_FALSE(NearlyEqual(a * b, b * a));
}

TEST(QuaternionProductTest, IsAssociative)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();
    auto c = Euler(0.5f, 0.5f, -0.5f);

    EXPECT_TRUE(NearlyEqual((a * b) * c, a * (b * c)));
}

TEST(QuaternionProductTest, MultipliesLengths)
{
    Quaternion a{1.0f, 2.0f, 3.0f, 4.0f};
    Quaternion b{5.0f, -6.0f, 7.0f, 8.0f};

    EXPECT_NEAR(Length(a) * Length(b), Length(a * b), 1e-3f);
}

TEST(QuaternionProductTest, KeepsUnitQuaternionsUnit)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    EXPECT_TRUE(IsNormalized(a * b));
}

TEST(QuaternionProductTest, DistributesOverAddition)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();
    auto c = Euler(0.5f, 0.5f, -0.5f);

    EXPECT_TRUE(NearlyEqual(a * (b + c), a * b + a * c));
}

TEST(QuaternionConjugateTest, NegatesVectorPartOnly)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_TRUE(NearlyEqual(Quaternion{-1.0f, -2.0f, -3.0f, 4.0f}, Conjugate(q)));
}

TEST(QuaternionConjugateTest, AppliedTwiceReturnsOriginal)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(q, Conjugate(Conjugate(q))));
}

TEST(QuaternionConjugateTest, PreservesLength)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(LengthSquared(q), LengthSquared(Conjugate(q)));
}

TEST(QuaternionConjugateTest, ReversesProductOrder)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    EXPECT_TRUE(NearlyEqual(Conjugate(a * b), Conjugate(b) * Conjugate(a)));
}

TEST(QuaternionConjugateTest, TimesItselfGivesLengthSquared)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    auto product = q * Conjugate(q);

    EXPECT_NEAR(LengthSquared(q), product.W, 1e-4f);
    EXPECT_NEAR(0.0f, product.X, 1e-4f);
    EXPECT_NEAR(0.0f, product.Y, 1e-4f);
    EXPECT_NEAR(0.0f, product.Z, 1e-4f);
}

TEST(QuaternionInverseTest, MatchesConjugateForUnitQuaternions)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(Conjugate(q), Inverse(q)));
}

TEST(QuaternionInverseTest, ProductWithInverseIsIdentity)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_TRUE(NearlyEqual(Identity, q * Inverse(q), 1e-4f));
    EXPECT_TRUE(NearlyEqual(Identity, Inverse(q) * q, 1e-4f));
}

TEST(QuaternionInverseTest, AppliedTwiceReturnsOriginal)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_TRUE(NearlyEqual(q, Inverse(Inverse(q)), 1e-4f));
}

TEST(QuaternionInverseTest, ScalesInverselyWithMagnitude)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(Inverse(q) * 0.5f, Inverse(q * 2.0f)));
}

TEST(QuaternionDotTest, SelfDotEqualsLengthSquared)
{
    Quaternion q{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(LengthSquared(q), Dot(q, q));
}

TEST(QuaternionDotTest, IsCommutative)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    EXPECT_FLOAT_EQ(Dot(a, b), Dot(b, a));
}

TEST(QuaternionDotTest, OrthogonalBasisElementsGiveZero)
{
    EXPECT_FLOAT_EQ(0.0f, Dot(I, J));
    EXPECT_FLOAT_EQ(0.0f, Dot(J, K));
    EXPECT_FLOAT_EQ(0.0f, Dot(K, Identity));
}

TEST(QuaternionDotTest, OppositeQuaternionsGiveNegativeLengthSquared)
{
    auto q = MakeArbitrary();

    EXPECT_FLOAT_EQ(-1.0f, Dot(q, -q));
}

TEST(QuaternionFromAxisAngleTest, ZeroAngleGivesIdentity)
{
    EXPECT_TRUE(NearlyEqual(Identity, FromAxisAngle(vec3::UnitZ, 0.0f)));
}

TEST(QuaternionFromAxisAngleTest, EncodesHalfAngle)
{
    auto q = FromAxisAngle(vec3::UnitZ, HalfPi);

    EXPECT_NEAR(0.0f, q.X, Tolerance);
    EXPECT_NEAR(0.0f, q.Y, Tolerance);
    EXPECT_NEAR(std::sin(QuarterPi), q.Z, Tolerance);
    EXPECT_NEAR(std::cos(QuarterPi), q.W, Tolerance);
}

TEST(QuaternionFromAxisAngleTest, ProducesUnitQuaternion)
{
    EXPECT_TRUE(IsNormalized(FromAxisAngle(vec3::UnitX, 1.3f)));
    EXPECT_TRUE(IsNormalized(FromAxisAngle(Vec3{1.0f, 2.0f, 3.0f}, -2.7f)));
}

TEST(QuaternionFromAxisAngleTest, NormalizesTheAxis)
{
    auto unit   = FromAxisAngle(vec3::UnitZ, 0.7f);
    auto scaled = FromAxisAngle(Vec3{0.0f, 0.0f, 5.0f}, 0.7f);

    EXPECT_TRUE(NearlyEqual(unit, scaled));
}

TEST(QuaternionFromAxisAngleTest, FullTurnGivesNegatedIdentity)
{
    auto q = FromAxisAngle(vec3::UnitZ, 2.0f * Pi);

    EXPECT_TRUE(NearlyEqual(-Identity, q, 1e-4f));
}

TEST(QuaternionFromAxisAngleTest, OppositeAngleGivesConjugate)
{
    auto q = FromAxisAngle(vec3::UnitY, 0.9f);

    EXPECT_TRUE(NearlyEqual(Conjugate(q), FromAxisAngle(vec3::UnitY, -0.9f)));
}

TEST(QuaternionEulerTest, ZeroAnglesGiveIdentity)
{
    EXPECT_TRUE(NearlyEqual(Identity, Euler(0.0f, 0.0f, 0.0f)));
}

TEST(QuaternionEulerTest, SingleAxisMatchesFromAxisAngle)
{
    EXPECT_TRUE(NearlyEqual(FromAxisAngle(vec3::UnitX, 0.8f), Euler(0.8f, 0.0f, 0.0f)));
    EXPECT_TRUE(NearlyEqual(FromAxisAngle(vec3::UnitY, 0.8f), Euler(0.0f, 0.8f, 0.0f)));
    EXPECT_TRUE(NearlyEqual(FromAxisAngle(vec3::UnitZ, 0.8f), Euler(0.0f, 0.0f, 0.8f)));
}

TEST(QuaternionEulerTest, AppliesRotationsInXThenYThenZOrder)
{
    const float x = 0.3f;
    const float y = -0.7f;
    const float z = 1.1f;

    auto composed =
        FromAxisAngle(vec3::UnitZ, z) * FromAxisAngle(vec3::UnitY, y) * FromAxisAngle(vec3::UnitX, x);

    EXPECT_TRUE(NearlyEqual(composed, Euler(x, y, z)));
}

TEST(QuaternionEulerTest, ProducesUnitQuaternion)
{
    EXPECT_TRUE(IsNormalized(MakeArbitrary()));
    EXPECT_TRUE(IsNormalized(Euler(3.0f, -3.0f, 3.0f)));
}

TEST(QuaternionRotateVectorTest, IdentityLeavesVectorUnchanged)
{
    auto v = MakeArbitraryVector();

    EXPECT_TRUE(math::NearlyEqual(v, RotateVector(Identity, v), Tolerance));
}

TEST(QuaternionRotateVectorTest, TurnsAxesByNinetyDegreesAboutZ)
{
    auto q = FromAxisAngle(vec3::UnitZ, HalfPi);

    EXPECT_TRUE(math::NearlyEqual(vec3::UnitY, RotateVector(q, vec3::UnitX), Tolerance));
    EXPECT_TRUE(math::NearlyEqual(Vec3{-1.0f, 0.0f, 0.0f}, RotateVector(q, vec3::UnitY), Tolerance));
    EXPECT_TRUE(math::NearlyEqual(vec3::UnitZ, RotateVector(q, vec3::UnitZ), Tolerance));
}

TEST(QuaternionRotateVectorTest, TurnsAxesByNinetyDegreesAboutXAndY)
{
    auto aboutX = FromAxisAngle(vec3::UnitX, HalfPi);
    auto aboutY = FromAxisAngle(vec3::UnitY, HalfPi);

    EXPECT_TRUE(math::NearlyEqual(vec3::UnitZ, RotateVector(aboutX, vec3::UnitY), Tolerance));
    EXPECT_TRUE(math::NearlyEqual(vec3::UnitX, RotateVector(aboutY, vec3::UnitZ), Tolerance));
}

TEST(QuaternionRotateVectorTest, LeavesTheRotationAxisFixed)
{
    auto axis = vec::Normalize(Vec3{1.0f, 2.0f, 3.0f});
    auto q    = FromAxisAngle(axis, 1.4f);

    EXPECT_TRUE(math::NearlyEqual(axis, RotateVector(q, axis), Tolerance));
}

TEST(QuaternionRotateVectorTest, PreservesLength)
{
    auto q = MakeArbitrary();
    auto v = MakeArbitraryVector();

    EXPECT_NEAR(vec::Length(v), vec::Length(RotateVector(q, v)), Tolerance);
}

TEST(QuaternionRotateVectorTest, ComposesInTheSameOrderAsTheProduct)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();
    auto v = MakeArbitraryVector();

    EXPECT_TRUE(
        math::NearlyEqual(RotateVector(a * b, v), RotateVector(a, RotateVector(b, v)), Tolerance)
    );
}

TEST(QuaternionRotateVectorTest, ConjugateUndoesTheRotation)
{
    auto q = MakeArbitrary();
    auto v = MakeArbitraryVector();

    EXPECT_TRUE(math::NearlyEqual(v, RotateVector(Conjugate(q), RotateVector(q, v)), Tolerance));
}

TEST(QuaternionRotateVectorTest, OppositeQuaternionsRotateAlike)
{
    auto q = MakeArbitrary();
    auto v = MakeArbitraryVector();

    EXPECT_TRUE(math::NearlyEqual(RotateVector(q, v), RotateVector(-q, v), Tolerance));
}

TEST(QuaternionToMatrixTest, IdentityGivesIdentityMatrices)
{
    EXPECT_TRUE(math::NearlyEqual(mat3::Identity, ToMat3(Identity), Tolerance));
    EXPECT_TRUE(math::NearlyEqual(mat4::Identity, ToMat4(Identity), Tolerance));
}

TEST(QuaternionToMatrixTest, Mat3AgreesWithRotateVector)
{
    auto q = MakeArbitrary();
    auto v = MakeArbitraryVector();

    EXPECT_TRUE(math::NearlyEqual(RotateVector(q, v), ToMat3(q) * v, Tolerance));
}

TEST(QuaternionToMatrixTest, Mat3IsARotation)
{
    auto m = ToMat3(MakeArbitrary());

    EXPECT_NEAR(1.0f, mat::Determinant(m), 1e-4f);
    EXPECT_TRUE(math::NearlyEqual(mat3::Identity, mat::Transpose(m) * m, 1e-4f));
}

TEST(QuaternionToMatrixTest, ConjugateGivesTransposedMatrix)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(math::NearlyEqual(mat::Transpose(ToMat3(q)), ToMat3(Conjugate(q)), Tolerance));
}

TEST(QuaternionToMatrixTest, ProductMapsToMatrixProduct)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    EXPECT_TRUE(math::NearlyEqual(ToMat3(a) * ToMat3(b), ToMat3(a * b), 1e-4f));
}

TEST(QuaternionToMatrixTest, EulerAgreesWithMatrixRotateEuler)
{
    auto q = Euler(0.3f, -0.7f, 1.1f);
    auto v = MakeArbitraryVector();

    auto expected = mat4::TransformDirection(mat4::RotateEuler(0.3f, -0.7f, 1.1f), v);

    EXPECT_TRUE(math::NearlyEqual(expected, RotateVector(q, v), 1e-4f));
}

TEST(QuaternionToMatrixTest, Mat4EmbedsMat3InTheUpperLeftBlock)
{
    auto q  = MakeArbitrary();
    auto m3 = ToMat3(q);
    auto m4 = ToMat4(q);

    for (usize row = 0; row < 3; ++row)
    {
        for (usize column = 0; column < 3; ++column)
        {
            EXPECT_FLOAT_EQ(mat::At(m3, row, column), mat::At(m4, row, column))
                << "row " << row << " column " << column;
        }
    }
}

TEST(QuaternionToMatrixTest, Mat4HasNoTranslationOrProjection)
{
    auto m = ToMat4(MakeArbitrary());

    for (usize i = 0; i < 3; ++i)
    {
        EXPECT_FLOAT_EQ(0.0f, mat::At(m, i, 3)) << "row " << i;
        EXPECT_FLOAT_EQ(0.0f, mat::At(m, 3, i)) << "column " << i;
    }

    EXPECT_FLOAT_EQ(1.0f, mat::At(m, 3, 3));
}

TEST(QuaternionToMatrixTest, Mat4TransformsPointsAndDirectionsAlike)
{
    auto q = MakeArbitrary();
    auto v = MakeArbitraryVector();
    auto m = ToMat4(q);

    EXPECT_TRUE(math::NearlyEqual(RotateVector(q, v), mat4::TransformPoint(m, v), Tolerance));
    EXPECT_TRUE(math::NearlyEqual(RotateVector(q, v), mat4::TransformDirection(m, v), Tolerance));
}

TEST(QuaternionToMatrixTest, Mat4ComposesWithTranslation)
{
    auto q           = MakeArbitrary();
    auto v           = MakeArbitraryVector();
    auto translation = Vec3{5.0f, 6.0f, 7.0f};

    auto transform = mat4::Translation(translation) * ToMat4(q);

    EXPECT_TRUE(math::NearlyEqual(
        RotateVector(q, v) + translation, mat4::TransformPoint(transform, v), Tolerance
    ));
}

TEST(QuaternionSlerpTest, ReturnsEndpointsAtZeroAndOne)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    EXPECT_TRUE(NearlyEqual(a, Slerp(a, b, 0.0f)));
    EXPECT_TRUE(NearlyEqual(b, Slerp(a, b, 1.0f)));
}

TEST(QuaternionSlerpTest, StaysNormalized)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    for (float t = 0.0f; t <= 1.0f; t += 0.1f)
    {
        EXPECT_TRUE(IsNormalized(Slerp(a, b, t))) << "t " << t;
    }
}

TEST(QuaternionSlerpTest, MovesAtConstantAngularVelocity)
{
    auto a = Identity;
    auto b = FromAxisAngle(vec3::UnitZ, HalfPi);

    EXPECT_TRUE(NearlyEqual(FromAxisAngle(vec3::UnitZ, HalfPi * 0.25f), Slerp(a, b, 0.25f)));
    EXPECT_TRUE(NearlyEqual(FromAxisAngle(vec3::UnitZ, HalfPi * 0.50f), Slerp(a, b, 0.50f)));
    EXPECT_TRUE(NearlyEqual(FromAxisAngle(vec3::UnitZ, HalfPi * 0.75f), Slerp(a, b, 0.75f)));
}

TEST(QuaternionSlerpTest, IsSymmetricInItsEndpoints)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    EXPECT_TRUE(NearlyEqual(Slerp(a, b, 0.3f), Slerp(b, a, 0.7f)));
}

TEST(QuaternionSlerpTest, TakesTheShortestPath)
{
    auto a = MakeArbitrary();
    auto b = MakeOtherArbitrary();

    EXPECT_TRUE(NearlyEqual(Slerp(a, b, 0.4f), Slerp(a, -b, 0.4f)));
}

TEST(QuaternionSlerpTest, HandlesIdenticalEndpoints)
{
    auto q = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(q, Slerp(q, q, 0.0f)));
    EXPECT_TRUE(NearlyEqual(q, Slerp(q, q, 0.5f)));
    EXPECT_TRUE(NearlyEqual(q, Slerp(q, q, 1.0f)));
}

TEST(QuaternionSlerpTest, FallsBackToLinearPathForNearbyQuaternions)
{
    auto a = Identity;
    auto b = FromAxisAngle(vec3::UnitZ, 0.001f);

    auto midpoint = Slerp(a, b, 0.5f);

    EXPECT_TRUE(IsNormalized(midpoint));
    EXPECT_TRUE(NearlyEqual(FromAxisAngle(vec3::UnitZ, 0.0005f), midpoint));
}

TEST(QuaternionSlerpTest, InterpolatedRotationActsOnVectors)
{
    auto a = Identity;
    auto b = FromAxisAngle(vec3::UnitZ, HalfPi);

    auto rotated = RotateVector(Slerp(a, b, 0.5f), vec3::UnitX);

    EXPECT_TRUE(
        math::NearlyEqual(Vec3{std::cos(QuarterPi), std::sin(QuarterPi), 0.0f}, rotated, Tolerance)
    );
}

namespace
{
    static_assert(sizeof(Quaternion) == 4 * sizeof(float));

    static_assert(Identity.Data[0] == 0.0f);
    static_assert(Identity.Data[1] == 0.0f);
    static_assert(Identity.Data[2] == 0.0f);
    static_assert(Identity.Data[3] == 1.0f);
}
