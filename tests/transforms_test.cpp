#include <gtest/gtest.h>

#include <math/math_utils.h>
#include <math/matrix.h>
#include <math/quat.h>
#include <math/transforms.h>
#include <math/vector.h>
#include <world/coordinate_system.h>

#include <cmath>

using namespace cw;
using namespace cw::math;

namespace
{
    constexpr float HalfPi = Pi * 0.5f;
    constexpr float QuarterPi = Pi * 0.25f;

    constexpr float Tolerance = 1e-5f;

    Vec3 MakeArbitraryPoint()
    {
        return Vec3{2.0f, -3.0f, 4.0f};
    }
}

TEST(TransformPointTest, TranslationMovesPointsButNotDirections)
{
    const Mat4 translation = mat4::Translation(Vec3{1.0f, 1.0f, 1.0f});

    const Vec3 sample{2.0f, 3.0f, 4.0f};

    EXPECT_TRUE(NearlyEqual(Vec3{3.0f, 4.0f, 5.0f}, mat4::TransformPoint(translation, sample)));
    EXPECT_TRUE(NearlyEqual(sample, mat4::TransformDirection(translation, sample)));
}

TEST(TransformPointTest, LinearTransformsActAlikeOnPointsAndDirections)
{
    const Mat4 scale = mat4::Scale(Vec3{2.0f, 3.0f, 4.0f});

    const Vec3 sample = MakeArbitraryPoint();

    EXPECT_TRUE(NearlyEqual(
        mat4::TransformPoint(scale, sample),
        mat4::TransformDirection(scale, sample)
    ));
}

TEST(TransformPointTest, ProjectPointMatchesTransformPointForAffineMatrices)
{
    const Mat4 matrix = mat4::Translation(Vec3{1.0f, 2.0f, 3.0f}) * mat4::Scale(2.0f);

    const Vec3 sample = MakeArbitraryPoint();

    EXPECT_TRUE(NearlyEqual(
        mat4::TransformPoint(matrix, sample),
        mat4::ProjectPoint(matrix, sample),
        Tolerance
    ));
}

TEST(TransformPointTest, ProjectPointDividesByW)
{
    const Mat4 projection = cam::PerspectiveRH_ZO(HalfPi, 1.0f, 1.0f, 100.0f);

    const Vec3 sample{3.0f, -6.0f, -3.0f};

    const Vec4 raw = projection * Vec4{sample.X, sample.Y, sample.Z, 1.0f};
    const Vec3 projected = mat4::ProjectPoint(projection, sample);

    EXPECT_FLOAT_EQ(3.0f, raw.W);
    EXPECT_NEAR(raw.X / raw.W, projected.X, Tolerance);
    EXPECT_NEAR(raw.Y / raw.W, projected.Y, Tolerance);
    EXPECT_NEAR(raw.Z / raw.W, projected.Z, Tolerance);
}

TEST(TranslationTest, StoresTheOffsetInTheLastColumn)
{
    const Mat4 translation = mat4::Translation(Vec3{1.0f, 2.0f, 3.0f});

    EXPECT_FLOAT_EQ(1.0f, mat::At(translation, 0, 3));
    EXPECT_FLOAT_EQ(2.0f, mat::At(translation, 1, 3));
    EXPECT_FLOAT_EQ(3.0f, mat::At(translation, 2, 3));
    EXPECT_FLOAT_EQ(1.0f, mat::At(translation, 3, 3));
}

TEST(TranslationTest, ZeroOffsetGivesIdentity)
{
    EXPECT_TRUE(NearlyEqual(mat4::Identity, mat4::Translation(vec3::Zero)));
}

TEST(TranslationTest, ComposesByAddingOffsets)
{
    const Vec3 first{1.0f, 2.0f, 3.0f};
    const Vec3 second{-4.0f, 0.5f, 1.0f};

    EXPECT_TRUE(NearlyEqual(
        mat4::Translation(first + second),
        mat4::Translation(first) * mat4::Translation(second),
        Tolerance
    ));
}

TEST(TranslationTest, OppositeOffsetUndoesIt)
{
    const Vec3 offset{1.0f, 2.0f, 3.0f};

    EXPECT_TRUE(NearlyEqual(
        mat4::Identity,
        mat4::Translation(offset) * mat4::Translation(offset * -1.0f),
        Tolerance
    ));
}

TEST(ScaleTest, StoresFactorsOnTheDiagonal)
{
    const Mat4 scale = mat4::Scale(Vec3{2.0f, 3.0f, 4.0f});

    EXPECT_FLOAT_EQ(2.0f, mat::At(scale, 0, 0));
    EXPECT_FLOAT_EQ(3.0f, mat::At(scale, 1, 1));
    EXPECT_FLOAT_EQ(4.0f, mat::At(scale, 2, 2));
    EXPECT_FLOAT_EQ(1.0f, mat::At(scale, 3, 3));
}

TEST(ScaleTest, ScalesEveryComponentOfAPoint)
{
    const Mat4 scale = mat4::Scale(Vec3{2.0f, 3.0f, 4.0f});

    EXPECT_TRUE(NearlyEqual(
        Vec3{4.0f, -9.0f, 16.0f},
        mat4::TransformPoint(scale, MakeArbitraryPoint())
    ));
}

TEST(ScaleTest, UniformOverloadMatchesTheVectorOverload)
{
    EXPECT_TRUE(NearlyEqual(mat4::Scale(Vec3{2.5f, 2.5f, 2.5f}), mat4::Scale(2.5f)));
}

TEST(ScaleTest, OneGivesIdentity)
{
    EXPECT_TRUE(NearlyEqual(mat4::Identity, mat4::Scale(1.0f)));
}

TEST(ScaleTest, ComposesByMultiplyingFactors)
{
    EXPECT_TRUE(NearlyEqual(
        mat4::Scale(Vec3{2.0f, 6.0f, 12.0f}),
        mat4::Scale(Vec3{1.0f, 2.0f, 3.0f}) * mat4::Scale(Vec3{2.0f, 3.0f, 4.0f}),
        Tolerance
    ));
}

TEST(RotationTest, ZeroAngleGivesIdentity)
{
    EXPECT_TRUE(NearlyEqual(mat4::Identity, mat4::RotationX(0.0f), Tolerance));
    EXPECT_TRUE(NearlyEqual(mat4::Identity, mat4::RotationY(0.0f), Tolerance));
    EXPECT_TRUE(NearlyEqual(mat4::Identity, mat4::RotationZ(0.0f), Tolerance));
}

TEST(RotationTest, TurnsAxesByAQuarterTurn)
{
    EXPECT_TRUE(NearlyEqual(
        vec3::UnitY,
        mat4::TransformDirection(mat4::RotationZ(HalfPi), vec3::UnitX),
        1e-5f
    ));

    EXPECT_TRUE(NearlyEqual(
        vec3::UnitZ,
        mat4::TransformDirection(mat4::RotationX(HalfPi), vec3::UnitY),
        1e-5f
    ));

    EXPECT_TRUE(NearlyEqual(
        vec3::UnitX,
        mat4::TransformDirection(mat4::RotationY(HalfPi), vec3::UnitZ),
        1e-5f
    ));
}

TEST(RotationTest, LeavesItsOwnAxisFixed)
{
    EXPECT_TRUE(NearlyEqual(
        vec3::UnitZ,
        mat4::TransformDirection(mat4::RotationZ(1.3f), vec3::UnitZ),
        1e-5f
    ));

    EXPECT_TRUE(NearlyEqual(
        vec3::UnitX,
        mat4::TransformDirection(mat4::RotationX(1.3f), vec3::UnitX),
        1e-5f
    ));

    EXPECT_TRUE(NearlyEqual(
        vec3::UnitY,
        mat4::TransformDirection(mat4::RotationY(1.3f), vec3::UnitY),
        1e-5f
    ));
}

TEST(RotationTest, PreservesLength)
{
    const Vec3 sample = MakeArbitraryPoint();

    const Vec3 rotated = mat4::TransformDirection(mat4::RotateEuler(0.4f, -1.1f, 2.0f), sample);

    EXPECT_NEAR(vec::Length(sample), vec::Length(rotated), 1e-4f);
}

TEST(RotationTest, IsOrthonormalWithUnitDeterminant)
{
    const Mat4 rotation = mat4::RotateEuler(0.4f, -1.1f, 2.0f);

    EXPECT_TRUE(NearlyEqual(mat4::Identity, rotation * mat::Transpose(rotation), 1e-4f));
    EXPECT_NEAR(1.0f, mat::Determinant(rotation), 1e-4f);
}

TEST(RotationTest, ComposesByAddingAnglesAboutTheSameAxis)
{
    EXPECT_TRUE(NearlyEqual(
        mat4::RotationZ(0.9f),
        mat4::RotationZ(0.4f) * mat4::RotationZ(0.5f),
        1e-5f
    ));

    EXPECT_TRUE(NearlyEqual(
        mat4::RotationY(0.9f),
        mat4::RotationY(0.4f) * mat4::RotationY(0.5f),
        1e-5f
    ));
}

TEST(RotationTest, OppositeAngleUndoesTheRotation)
{
    EXPECT_TRUE(NearlyEqual(
        mat4::Identity,
        mat4::RotationX(1.2f) * mat4::RotationX(-1.2f),
        1e-5f
    ));
}

TEST(RotationTest, MatchesTheQuaternionMatrix)
{
    EXPECT_TRUE(NearlyEqual(
        mat4::RotationX(0.7f),
        quat::ToMat4(quat::FromAxisAngle(vec3::UnitX, 0.7f)),
        1e-5f
    ));

    EXPECT_TRUE(NearlyEqual(
        mat4::RotationY(0.7f),
        quat::ToMat4(quat::FromAxisAngle(vec3::UnitY, 0.7f)),
        1e-5f
    ));

    EXPECT_TRUE(NearlyEqual(
        mat4::RotationZ(0.7f),
        quat::ToMat4(quat::FromAxisAngle(vec3::UnitZ, 0.7f)),
        1e-5f
    ));
}

TEST(RotationTest, EulerAppliesXThenYThenZ)
{
    const float x = 0.4f;
    const float y = -1.1f;
    const float z = 2.0f;

    EXPECT_TRUE(NearlyEqual(
        mat4::RotationZ(z) * mat4::RotationY(y) * mat4::RotationX(x),
        mat4::RotateEuler(x, y, z),
        1e-5f
    ));
}

TEST(RotationTest, EulerMatchesTheQuaternionEuler)
{
    const float x = 0.4f;
    const float y = -1.1f;
    const float z = 2.0f;

    EXPECT_TRUE(NearlyEqual(
        mat4::RotateEuler(x, y, z),
        quat::ToMat4(quat::Euler(x, y, z)),
        1e-4f
    ));
}

TEST(TrsTest, IdentityPartsGiveIdentity)
{
    EXPECT_TRUE(NearlyEqual(
        mat4::Identity,
        mat4::TRS(vec3::Zero, quat::Identity, vec3::One),
        Tolerance
    ));
}

TEST(TrsTest, KeepsTranslationInTheLastColumn)
{
    const Vec3 translation{1.0f, 2.0f, 3.0f};

    const Mat4 matrix = mat4::TRS(translation, quat::Euler(0.3f, 0.4f, 0.5f), Vec3{2.0f, 2.0f, 2.0f});

    EXPECT_FLOAT_EQ(1.0f, mat::At(matrix, 0, 3));
    EXPECT_FLOAT_EQ(2.0f, mat::At(matrix, 1, 3));
    EXPECT_FLOAT_EQ(3.0f, mat::At(matrix, 2, 3));
}

TEST(TrsTest, AppliesScaleThenRotationThenTranslation)
{
    const Vec3 translation{1.0f, -2.0f, 0.5f};
    const Quaternion rotation = quat::Euler(0.3f, 0.4f, 0.5f);
    const Vec3 scale{2.0f, 3.0f, 4.0f};

    const Vec3 sample = MakeArbitraryPoint();

    const Vec3 scaled{sample.X * scale.X, sample.Y * scale.Y, sample.Z * scale.Z};
    const Vec3 expected = quat::RotateVector(rotation, scaled) + translation;

    EXPECT_TRUE(NearlyEqual(
        expected,
        mat4::TransformPoint(mat4::TRS(translation, rotation, scale), sample),
        1e-4f
    ));
}

TEST(TrsTest, DirectionsIgnoreTheTranslation)
{
    const Quaternion rotation = quat::Euler(0.3f, 0.4f, 0.5f);

    const Mat4 withTranslation = mat4::TRS(Vec3{5.0f, 6.0f, 7.0f}, rotation, vec3::One);
    const Mat4 withoutTranslation = mat4::TRS(vec3::Zero, rotation, vec3::One);

    const Vec3 sample = MakeArbitraryPoint();

    EXPECT_TRUE(NearlyEqual(
        mat4::TransformDirection(withoutTranslation, sample),
        mat4::TransformDirection(withTranslation, sample),
        1e-5f
    ));
}

TEST(TrsTest, MatchesTheProductOfItsParts)
{
    const Vec3 translation{1.0f, -2.0f, 0.5f};
    const Quaternion rotation = quat::Euler(0.3f, 0.4f, 0.5f);
    const Vec3 scale{2.0f, 3.0f, 4.0f};

    const Mat4 expected =
        mat4::Translation(translation) * quat::ToMat4(rotation) * mat4::Scale(scale);

    EXPECT_TRUE(NearlyEqual(expected, mat4::TRS(translation, rotation, scale)));
}

TEST(LookAtTest, MapsTheEyeToTheOrigin)
{
    const Vec3 eye{3.0f, 4.0f, 5.0f};

    const Mat4 view = cam::LookAt(eye, vec3::Zero, world::Up);

    EXPECT_TRUE(NearlyZero(mat4::TransformPoint(view, eye), 1e-4f));
}

TEST(LookAtTest, PutsTheTargetOnTheNegativeZAxis)
{
    const Vec3 eye{0.0f, 0.0f, 5.0f};
    const Vec3 target{1.0f, 2.0f, -3.0f};

    const Mat4 view = cam::LookAt(eye, target, world::Up);

    const Vec3 transformed = mat4::TransformPoint(view, target);

    EXPECT_NEAR(0.0f, transformed.X, 1e-4f);
    EXPECT_NEAR(0.0f, transformed.Y, 1e-4f);
    EXPECT_NEAR(-vec::Distance(eye, target), transformed.Z, 1e-4f);
}

TEST(LookAtTest, LooksDownNegativeZForACameraOnThePositiveZAxis)
{
    const Mat4 view = cam::LookAt(Vec3{0.0f, 0.0f, 5.0f}, vec3::Zero, world::Up);

    EXPECT_TRUE(NearlyEqual(Vec3{0.0f, 0.0f, -5.0f}, mat4::TransformPoint(view, vec3::Zero), 1e-5f));
    EXPECT_TRUE(NearlyEqual(Vec3{1.0f, 0.0f, -5.0f}, mat4::TransformPoint(view, vec3::UnitX), 1e-5f));
    EXPECT_TRUE(NearlyEqual(Vec3{0.0f, 1.0f, -5.0f}, mat4::TransformPoint(view, vec3::UnitY), 1e-5f));
}

TEST(LookAtTest, KeepsTheUpDirectionUp)
{
    const Mat4 view = cam::LookAt(Vec3{4.0f, 2.0f, 6.0f}, Vec3{1.0f, 0.0f, 0.0f}, world::Up);

    const Vec3 up = mat4::TransformDirection(view, world::Up);

    EXPECT_GT(up.Y, 0.0f);
    EXPECT_NEAR(0.0f, up.X, 1e-4f);
}

TEST(LookAtTest, IsARigidTransform)
{
    const Mat4 view = cam::LookAt(Vec3{4.0f, 2.0f, 6.0f}, Vec3{1.0f, 0.0f, 0.0f}, world::Up);

    const Vec3 first{1.0f, 2.0f, 3.0f};
    const Vec3 second{-2.0f, 0.5f, 1.0f};

    EXPECT_NEAR(
        vec::Distance(first, second),
        vec::Distance(mat4::TransformPoint(view, first), mat4::TransformPoint(view, second)),
        1e-4f
    );

    EXPECT_NEAR(1.0f, mat::Determinant(view), 1e-4f);
}

TEST(LookAtTest, RowsFormAnOrthonormalBasis)
{
    const Mat4 view = cam::LookAt(Vec3{4.0f, 2.0f, 6.0f}, Vec3{1.0f, 0.0f, 0.0f}, world::Up);

    const Vec3 right{mat::At(view, 0, 0), mat::At(view, 0, 1), mat::At(view, 0, 2)};
    const Vec3 up{mat::At(view, 1, 0), mat::At(view, 1, 1), mat::At(view, 1, 2)};
    const Vec3 back{mat::At(view, 2, 0), mat::At(view, 2, 1), mat::At(view, 2, 2)};

    EXPECT_NEAR(1.0f, vec::Length(right), 1e-5f);
    EXPECT_NEAR(1.0f, vec::Length(up), 1e-5f);
    EXPECT_NEAR(1.0f, vec::Length(back), 1e-5f);

    EXPECT_NEAR(0.0f, vec::Dot(right, up), 1e-5f);
    EXPECT_NEAR(0.0f, vec::Dot(right, back), 1e-5f);
    EXPECT_NEAR(0.0f, vec::Dot(up, back), 1e-5f);

    EXPECT_TRUE(NearlyEqual(back, vec::Cross(right, up), 1e-4f));
}

TEST(PerspectiveTest, ZeroToOneMapsNearToZeroAndFarToOne)
{
    const Mat4 projection = cam::PerspectiveRH_ZO(HalfPi, 1.0f, 1.0f, 100.0f);

    EXPECT_NEAR(0.0f, mat4::ProjectPoint(projection, Vec3{0.0f, 0.0f, -1.0f}).Z, 1e-5f);
    EXPECT_NEAR(1.0f, mat4::ProjectPoint(projection, Vec3{0.0f, 0.0f, -100.0f}).Z, 1e-5f);
}

TEST(PerspectiveTest, NegativeOneToOneMapsNearToMinusOneAndFarToOne)
{
    const Mat4 projection = cam::PerspectiveRH_NO(HalfPi, 1.0f, 1.0f, 100.0f);

    EXPECT_NEAR(-1.0f, mat4::ProjectPoint(projection, Vec3{0.0f, 0.0f, -1.0f}).Z, 1e-5f);
    EXPECT_NEAR(1.0f, mat4::ProjectPoint(projection, Vec3{0.0f, 0.0f, -100.0f}).Z, 1e-5f);
}

TEST(PerspectiveTest, KeepsTheCenterOfTheViewCentered)
{
    const Mat4 projection = cam::PerspectiveRH_ZO(HalfPi, 1.6f, 0.1f, 50.0f);

    const Vec3 projected = mat4::ProjectPoint(projection, Vec3{0.0f, 0.0f, -10.0f});

    EXPECT_NEAR(0.0f, projected.X, 1e-5f);
    EXPECT_NEAR(0.0f, projected.Y, 1e-5f);
}

TEST(PerspectiveTest, PutsTheFrustumEdgeAtTheClipBoundary)
{
    const Mat4 projection = cam::PerspectiveRH_ZO(HalfPi, 1.0f, 1.0f, 100.0f);

    EXPECT_NEAR(1.0f, mat4::ProjectPoint(projection, Vec3{0.0f, 10.0f, -10.0f}).Y, 1e-5f);
    EXPECT_NEAR(-1.0f, mat4::ProjectPoint(projection, Vec3{-10.0f, 0.0f, -10.0f}).X, 1e-5f);
}

TEST(PerspectiveTest, NarrowsTheHorizontalFieldOfViewWithAspectRatio)
{
    const Mat4 projection = cam::PerspectiveRH_ZO(HalfPi, 2.0f, 1.0f, 100.0f);

    EXPECT_NEAR(1.0f, mat4::ProjectPoint(projection, Vec3{20.0f, 0.0f, -10.0f}).X, 1e-5f);
    EXPECT_NEAR(1.0f, mat4::ProjectPoint(projection, Vec3{0.0f, 10.0f, -10.0f}).Y, 1e-5f);
}

TEST(PerspectiveTest, CarriesNegatedDepthIntoW)
{
    const Mat4 projection = cam::PerspectiveRH_ZO(HalfPi, 1.0f, 1.0f, 100.0f);

    const Vec4 transformed = projection * Vec4{1.0f, 2.0f, -7.0f, 1.0f};

    EXPECT_FLOAT_EQ(7.0f, transformed.W);
    EXPECT_FLOAT_EQ(-1.0f, mat::At(projection, 3, 2));
    EXPECT_FLOAT_EQ(0.0f, mat::At(projection, 3, 3));
}

TEST(PerspectiveTest, BothDepthRangesShareTheSameHorizontalAndVerticalScale)
{
    const Mat4 zeroToOne = cam::PerspectiveRH_ZO(1.2f, 1.6f, 0.1f, 50.0f);
    const Mat4 negativeOneToOne = cam::PerspectiveRH_NO(1.2f, 1.6f, 0.1f, 50.0f);

    EXPECT_FLOAT_EQ(mat::At(zeroToOne, 0, 0), mat::At(negativeOneToOne, 0, 0));
    EXPECT_FLOAT_EQ(mat::At(zeroToOne, 1, 1), mat::At(negativeOneToOne, 1, 1));
}

TEST(PerspectiveTest, ShrinksDistantObjects)
{
    const Mat4 projection = cam::PerspectiveRH_ZO(HalfPi, 1.0f, 1.0f, 100.0f);

    const Vec3 closer = mat4::ProjectPoint(projection, Vec3{1.0f, 0.0f, -2.0f});
    const Vec3 farther = mat4::ProjectPoint(projection, Vec3{1.0f, 0.0f, -20.0f});

    EXPECT_GT(closer.X, farther.X);
    EXPECT_GT(farther.X, 0.0f);
}

TEST(OrthographicTest, NegativeOneToOneMapsTheBoxCornersToTheClipCube)
{
    const Mat4 projection = cam::OrthographicRH_NO(-2.0f, 2.0f, -1.0f, 1.0f, 1.0f, 5.0f);

    const Vec3 lower = mat4::ProjectPoint(projection, Vec3{-2.0f, -1.0f, -1.0f});
    const Vec3 upper = mat4::ProjectPoint(projection, Vec3{2.0f, 1.0f, -5.0f});

    EXPECT_TRUE(NearlyEqual(Vec3{-1.0f, -1.0f, -1.0f}, lower, 1e-5f));
    EXPECT_TRUE(NearlyEqual(Vec3{1.0f, 1.0f, 1.0f}, upper, 1e-5f));
}

TEST(OrthographicTest, ZeroToOneMapsNearToZeroAndFarToOne)
{
    const Mat4 projection = cam::OrthographicRH_ZO(-2.0f, 2.0f, -1.0f, 1.0f, 1.0f, 5.0f);

    EXPECT_NEAR(0.0f, mat4::ProjectPoint(projection, Vec3{0.0f, 0.0f, -1.0f}).Z, 1e-5f);
    EXPECT_NEAR(1.0f, mat4::ProjectPoint(projection, Vec3{0.0f, 0.0f, -5.0f}).Z, 1e-5f);
}

TEST(OrthographicTest, MapsTheBoxCenterToTheOrigin)
{
    const Mat4 projection = cam::OrthographicRH_NO(-2.0f, 6.0f, -1.0f, 3.0f, 1.0f, 5.0f);

    const Vec3 center{2.0f, 1.0f, -3.0f};

    EXPECT_TRUE(NearlyZero(mat4::ProjectPoint(projection, center), 1e-5f));
}

TEST(OrthographicTest, KeepsWAtOne)
{
    const Mat4 projection = cam::OrthographicRH_NO(-2.0f, 2.0f, -1.0f, 1.0f, 1.0f, 5.0f);

    const Vec4 transformed = projection * Vec4{1.0f, 2.0f, -3.0f, 1.0f};

    EXPECT_FLOAT_EQ(1.0f, transformed.W);
}

TEST(OrthographicTest, DoesNotShrinkDistantObjects)
{
    const Mat4 projection = cam::OrthographicRH_NO(-2.0f, 2.0f, -1.0f, 1.0f, 1.0f, 5.0f);

    const Vec3 closer = mat4::ProjectPoint(projection, Vec3{1.0f, 0.0f, -2.0f});
    const Vec3 farther = mat4::ProjectPoint(projection, Vec3{1.0f, 0.0f, -4.0f});

    EXPECT_FLOAT_EQ(closer.X, farther.X);
}

TEST(OrthographicTest, BothDepthRangesShareTheSameHorizontalAndVerticalScale)
{
    const Mat4 negativeOneToOne = cam::OrthographicRH_NO(-2.0f, 6.0f, -1.0f, 3.0f, 1.0f, 5.0f);
    const Mat4 zeroToOne = cam::OrthographicRH_ZO(-2.0f, 6.0f, -1.0f, 3.0f, 1.0f, 5.0f);

    for (usize row = 0; row < 2; ++row)
    {
        for (usize column = 0; column < 4; ++column)
        {
            EXPECT_FLOAT_EQ(
                mat::At(negativeOneToOne, row, column),
                mat::At(zeroToOne, row, column)
            );
        }
    }
}

TEST(OrthographicTest, IsInvertible)
{
    const Mat4 projection = cam::OrthographicRH_NO(-2.0f, 6.0f, -1.0f, 3.0f, 1.0f, 5.0f);

    Mat4 inverse = mat4::Zero;

    ASSERT_TRUE(mat::TryInverse(projection, inverse));

    const Vec3 sample{1.0f, 2.0f, -3.0f};

    EXPECT_TRUE(NearlyEqual(
        sample,
        mat4::TransformPoint(inverse, mat4::TransformPoint(projection, sample)),
        1e-4f
    ));
}
